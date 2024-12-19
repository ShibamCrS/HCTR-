#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

#define BPI 8

#include "../include/setup.h"
#include "../include/deoxysbc.h"
#include "../include/init.h"

//multiply by 2 over finite field
static inline BLOCK increment(BLOCK b) {
    const __m128i mask = _mm_set_epi32(135,1,1,1);
    __m128i t = _mm_srai_epi32(b, 31);
    t = _mm_and_si128(t, mask);
    t = _mm_shuffle_epi32(t, _MM_SHUFFLE(2,1,0,3));
    b = _mm_slli_epi32(b, 1);
    return _mm_xor_si128(b,t);
}
#define REDUCTION_POLYNOMIAL  _mm_set_epi32(0, 0, 0, 135)
#define accumulate_eight(x, y) { \
    x[0] = XOR(x[0], x[1]); \
    x[2] = XOR(x[2], x[3]); \
    x[4] = XOR(x[4], x[5]); \
    x[6] = XOR(x[6], x[7]); \
    x[0] = XOR(x[0], x[2]); \
    x[4] = XOR(x[4], x[6]); \
    y    = XOR(x[0], x[4]); \
}
static inline __m128i gf_2_128_double_eight(__m128i hash, __m128i x[8]) {
    __m128i tmp[8];
    tmp[0] = _mm_srli_epi64(hash, 56);
    tmp[1] = _mm_srli_epi64(x[0], 57);
    tmp[2] = _mm_srli_epi64(x[1], 58);
    tmp[3] = _mm_srli_epi64(x[2], 59);
    tmp[4] = _mm_srli_epi64(x[3], 60);
    tmp[5] = _mm_srli_epi64(x[4], 61);
    tmp[6] = _mm_srli_epi64(x[5], 62);
    tmp[7] = _mm_srli_epi64(x[6], 63);

    __m128i sum;
    accumulate_eight(tmp, sum);

    // ---------------------------------------------------------------------
    // sum = sum_high || sum_low
    // We have to take sum_high * 135 and XOR it to our XOR sum to have the
    // Reduction term. The 0x01 indicates that sum_high is used.
    // ---------------------------------------------------------------------

    __m128i mod =  _mm_clmulepi64_si128(sum, REDUCTION_POLYNOMIAL, 0x01);

    // Move sum_low to the upper 64-bit half
    __m128i sum_low = _mm_bslli_si128(sum, 8);

    tmp[0] = _mm_slli_epi64(hash, 8);
    tmp[1] = _mm_slli_epi64(x[0], 7);
    tmp[2] = _mm_slli_epi64(x[1], 6);
    tmp[3] = _mm_slli_epi64(x[2], 5);
    tmp[4] = _mm_slli_epi64(x[3], 4);
    tmp[5] = _mm_slli_epi64(x[4], 3);
    tmp[6] = _mm_slli_epi64(x[5], 2);
    tmp[7] = _mm_slli_epi64(x[6], 1);

    accumulate_eight(tmp, sum);
    sum = XOR(sum, sum_low);
    sum = XOR(sum, mod);
    sum = XOR(sum, x[7]);
    return sum;
}


int prp_encrypt(prp_ctx     * restrict ctx,
               const void *pt,
               int         pt_len,
               const void *tk,
               int         tk_len,
               void       *ct,
               int        encrypt)
{
    unsigned i, j, k, remaining=0, iters, npblks, index, local_len;
    BLOCK       * restrict ctp = (BLOCK *)ct;
    const BLOCK * restrict ptp = (BLOCK *)pt;
    const BLOCK * restrict tkp = (BLOCK *)tk;
    BLOCK X, Y, Z, W, ctr;
    BLOCK T, t, S;
    npblks = 16*BPI;

    __m128i ONE    = _mm_set_epi8( 0x4, 0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 ); 
    __m128i TWO    = _mm_set_epi8( 0x8, 0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 );
    __m128i THREE  = _mm_set_epi8( 0xC, 0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 );
    
    BLOCK RT[8][BPI];
    BLOCK States[BPI];

/* --------------- The Upper Hash using PMAC2x -------------------------*/
/* --------------------------------- Process Tweaks -------------------------*/
    local_len = tk_len;
    index = 0;
    iters = local_len/npblks;
    X = ZERO();
    Y = ZERO();
    ctr = ZERO();
    while (iters) {
        ctr =  ADD_ONE(ctr); RT[0][0] = ctr;
        ctr =  ADD_ONE(ctr); RT[0][1] = ctr;
        ctr =  ADD_ONE(ctr); RT[0][2] = ctr;
        ctr =  ADD_ONE(ctr); RT[0][3] = ctr;
        ctr =  ADD_ONE(ctr); RT[0][4] = ctr;
        ctr =  ADD_ONE(ctr); RT[0][5] = ctr;
        ctr =  ADD_ONE(ctr); RT[0][6] = ctr;
        ctr =  ADD_ONE(ctr); RT[0][7] = ctr;
            
        /* for(i=1; i<8; i++) UPDATE_TWEAK(RT, i); */
        for(i=1; i<8; i++){ //UPDATE_TWEAK 
            RT[i][0] = RT[i-1][0];
            RT[i][1] = RT[i-1][1];
            RT[i][2] = RT[i-1][2];
            RT[i][3] = RT[i-1][3];
            RT[i][4] = RT[i-1][4];
            RT[i][5] = RT[i-1][5];
            RT[i][6] = RT[i-1][6];
            RT[i][7] = RT[i-1][7];
        }
        States[0] = tkp[index  ];
        States[1] = tkp[index+1];
        States[2] = tkp[index+2];
        States[3] = tkp[index+3];
        States[4] = tkp[index+4];
        States[5] = tkp[index+5];
        States[6] = tkp[index+6];
        States[7] = tkp[index+7];
        DEOXYS( States, ctx->round_keys_h, RT )   
        
        X = XOR(X, States[0]);
        X = XOR(X, States[1]);
        X = XOR(X, States[2]);
        X = XOR(X, States[3]);
        X = XOR(X, States[4]);
        X = XOR(X, States[5]);
        X = XOR(X, States[6]);
        X = XOR(X, States[7]);
        Y = gf_2_128_double_eight(Y, States);
        /* Y = increment(Y);Y = XOR(Y, States[0]); */  
        /* Y = increment(Y);Y = XOR(Y, States[1]); */  
        /* Y = increment(Y);Y = XOR(Y, States[2]); */  
        /* Y = increment(Y);Y = XOR(Y, States[3]); */  
        /* Y = increment(Y);Y = XOR(Y, States[4]); */  
        /* Y = increment(Y);Y = XOR(Y, States[5]); */  
        /* Y = increment(Y);Y = XOR(Y, States[6]); */  
        /* Y = increment(Y);Y = XOR(Y, States[7]); */  

        index += BPI;
        --iters;
    }

    remaining = local_len % npblks;
    while (remaining >= 16) {
        ctr = ADD_ONE(ctr); T = ctr; S = tkp[index];
        TAES(S, ctx->round_keys_h, T, t);
        X = XOR(X, S);
        Y = increment(Y); Y = XOR(Y, S); 
        index += 1;
        remaining -= 16;
    }
    if (remaining > 0) { //If the last block is not full
        ctr = ADD_ONE(ctr); T = XOR(ctr, ONE); //DomainSep
        S = ZERO();
        memcpy(&S, tkp+index, remaining);
        ((unsigned char *)&S)[remaining] = (unsigned char)0x80; //padding
        TAES(S, ctx->round_keys_h, T, t);
        X = XOR(X, S);
        Y = increment(Y); Y = XOR(Y, S); 
        ctr = XOR(ctr, ONE); //recover counter
    }
    BLOCK HT0 = X;
    BLOCK HT1 = Y;
    BLOCK CTR_SAVE = ctr;
/*-----------------------Process Plaintexts----------------------------*/
    local_len = (pt_len - 2*16); //First two message block will be used later
    index = 2; //First two message block will be used later
    iters = local_len/npblks;
    ctr = XOR(ctr, TWO); //Domain Seperateation indication: plaintext domain
    while (iters) {
        ctr =  ADD_ONE(ctr); RT[0][0] = ctr;
        ctr =  ADD_ONE(ctr); RT[0][1] = ctr;
        ctr =  ADD_ONE(ctr); RT[0][2] = ctr;
        ctr =  ADD_ONE(ctr); RT[0][3] = ctr;
        ctr =  ADD_ONE(ctr); RT[0][4] = ctr;
        ctr =  ADD_ONE(ctr); RT[0][5] = ctr;
        ctr =  ADD_ONE(ctr); RT[0][6] = ctr;
        ctr =  ADD_ONE(ctr); RT[0][7] = ctr;
            
        /* for(i=1; i<8; i++) UPDATE_TWEAK(RT, i); */
        for(i=1; i<8; i++){ //UPDATE_TWEAK 
            RT[i][0] = RT[i-1][0];
            RT[i][1] = RT[i-1][1];
            RT[i][2] = RT[i-1][2];
            RT[i][3] = RT[i-1][3];
            RT[i][4] = RT[i-1][4];
            RT[i][5] = RT[i-1][5];
            RT[i][6] = RT[i-1][6];
            RT[i][7] = RT[i-1][7];
        }
        States[0] = ptp[index  ];
        States[1] = ptp[index+1];
        States[2] = ptp[index+2];
        States[3] = ptp[index+3];
        States[4] = ptp[index+4];
        States[5] = ptp[index+5];
        States[6] = ptp[index+6];
        States[7] = ptp[index+7];
        DEOXYS( States, ctx->round_keys_h, RT )   
        
        X = XOR(X, States[0]);
        X = XOR(X, States[1]);
        X = XOR(X, States[2]);
        X = XOR(X, States[3]);
        X = XOR(X, States[4]);
        X = XOR(X, States[5]);
        X = XOR(X, States[6]);
        X = XOR(X, States[7]);

        Y = gf_2_128_double_eight(Y, States);
        /* Y = increment(Y);Y = XOR(Y, States[0]); */  
        /* Y = increment(Y);Y = XOR(Y, States[1]); */  
        /* Y = increment(Y);Y = XOR(Y, States[2]); */  
        /* Y = increment(Y);Y = XOR(Y, States[3]); */  
        /* Y = increment(Y);Y = XOR(Y, States[4]); */  
        /* Y = increment(Y);Y = XOR(Y, States[5]); */  
        /* Y = increment(Y);Y = XOR(Y, States[6]); */  
        /* Y = increment(Y);Y = XOR(Y, States[7]); */  

        index += BPI;
        --iters;
    }

    remaining = local_len % npblks;
    while (remaining >= 16) {
        /* printf("Remaining = %d index = %d =============\n", remaining, index); */
        ctr = ADD_ONE(ctr); T = ctr;
        S = ptp[index];
        TAES(S, ctx->round_keys_h, T, t);
        X = XOR(X, S);
        Y = increment(Y); Y = XOR(Y, S); 
        index += 1;
        remaining -= 16;
    }
    if (remaining > 0) { //If the last block is not full
        ctr = ADD_ONE(ctr); T = XOR(ctr, ONE); //DomainSep
        S = ZERO();
        memcpy(&S, ptp+index, remaining);
        ((unsigned char *)&S)[remaining] = (unsigned char)0x80; //padding
        TAES(S, ctx->round_keys_h, T, t);
        X = XOR(X, S);
        Y = increment(Y);Y = XOR(Y, S);  
    }
    Y = increment(Y);
    //FInalization Of Hash
    Z = X; W = Y;
    Y = OR(Y, TWO); X = OR(X, THREE);
    TAES(Z, ctx->round_keys_h, Y, t);
    TAES(W, ctx->round_keys_h, X, t);
/*-----------------Process Upper Part Of The First Two Blocks----------------*/
    Z = XOR(ptp[0], Z);
    W = XOR(ptp[1], W);
    if(encrypt) {
#ifdef PRINT
    printf("U1: "); printreg(&Z, 16);
    printf("U2: "); printreg(&W, 16);
#endif
    TAES(Z, ctx->round_keys_1, W, t);
    TAES(W, ctx->round_keys_2, Z, t);
#ifdef PRINT
    printf("Z : "); printreg(&Z, 16);
    printf("W : "); printreg(&W, 16);
#endif
    S = Z; T = W;
    TAES(S, ctx->round_keys_3, T, t);
    TAES(T, ctx->round_keys_4, S, t);
#ifdef PRINT
    printf("V1: "); printreg(&S, 16);
    printf("V2: "); printreg(&T, 16);
#endif
    }
    else {
#ifdef PRINT
    printf("V1: "); printreg(&Z, 16);
    printf("V2: "); printreg(&W, 16);
#endif
    TAESD(W, ctx->round_keys_d_4, Z, t);
    TAESD(Z, ctx->round_keys_d_3, W, t);
#ifdef PRINT
    printf("Z : "); printreg(&Z, 16);
    printf("W : "); printreg(&W, 16);
#endif
    S = Z; T = W;
    TAESD(T, ctx->round_keys_d_2, S, t);
    TAESD(S, ctx->round_keys_d_1, T, t);
#ifdef PRINT
    printf("U1: "); printreg(&S, 16);
    printf("U2: "); printreg(&T, 16);
#endif
    }
    ctp[0] = S; ctp[1] = T;
/*------------------------------------ The CTR Part -------------------------*/
    local_len = (pt_len - 2*16); //First two message block will be used later
    iters = local_len/npblks;
    index = 2; //First two message block will be used later
    ctr = ZERO();
    while (iters) {
        ctr =  ADD_ONE(ctr); RT[0][0] = XOR(ctr, Z);
        ctr =  ADD_ONE(ctr); RT[0][1] = XOR(ctr, Z);
        ctr =  ADD_ONE(ctr); RT[0][2] = XOR(ctr, Z);
        ctr =  ADD_ONE(ctr); RT[0][3] = XOR(ctr, Z);
        ctr =  ADD_ONE(ctr); RT[0][4] = XOR(ctr, Z);
        ctr =  ADD_ONE(ctr); RT[0][5] = XOR(ctr, Z);
        ctr =  ADD_ONE(ctr); RT[0][6] = XOR(ctr, Z);
        ctr =  ADD_ONE(ctr); RT[0][7] = XOR(ctr, Z);
            
        for(i=1; i<8; i++){ //UPDATE_TWEAK 
            RT[i][0] = RT[i-1][0];
            RT[i][1] = RT[i-1][1];
            RT[i][2] = RT[i-1][2];
            RT[i][3] = RT[i-1][3];
            RT[i][4] = RT[i-1][4];
            RT[i][5] = RT[i-1][5];
            RT[i][6] = RT[i-1][6];
            RT[i][7] = RT[i-1][7];
        }
        States[0]=States[1]=States[2]=States[3]=States[4]=States[5]=States[6]=States[7]=W;
        DEOXYS( States, ctx->round_keys_c, RT )   
        
        ctp[index    ] = XOR(States[0], ptp[index    ]);
        ctp[index + 1] = XOR(States[1], ptp[index + 1]);
        ctp[index + 2] = XOR(States[2], ptp[index + 2]);
        ctp[index + 3] = XOR(States[3], ptp[index + 3]);
        ctp[index + 4] = XOR(States[4], ptp[index + 4]);
        ctp[index + 5] = XOR(States[5], ptp[index + 5]);
        ctp[index + 6] = XOR(States[6], ptp[index + 6]);
        ctp[index + 7] = XOR(States[7], ptp[index + 7]);

        index += BPI;
        --iters;
    }
    remaining = local_len % npblks;
    while (remaining >= 16) {
        ctr = ADD_ONE(ctr); T = XOR(ctr, Z);
        S = W;
        TAES(S, ctx->round_keys_c, T, t);
        ctp[index] = XOR(S, ptp[index]);
        index += 1;
        remaining -= 16;
    }
/* ---------------- The Lower Hash using PMAC2x -------------------------*/
/*----------------------- Process Plaintexts ----------------------------*/
    local_len = (pt_len - 2*16); //First two message block will be used later
    index = 2; //First two message block will be used later
    iters = local_len/npblks;
    ctr = XOR(CTR_SAVE, TWO); //Use Saved values from previous tweak process
    X = HT0;
    Y = HT1;
    while (iters) {
        ctr =  ADD_ONE(ctr); RT[0][0] = ctr;
        ctr =  ADD_ONE(ctr); RT[0][1] = ctr;
        ctr =  ADD_ONE(ctr); RT[0][2] = ctr;
        ctr =  ADD_ONE(ctr); RT[0][3] = ctr;
        ctr =  ADD_ONE(ctr); RT[0][4] = ctr;
        ctr =  ADD_ONE(ctr); RT[0][5] = ctr;
        ctr =  ADD_ONE(ctr); RT[0][6] = ctr;
        ctr =  ADD_ONE(ctr); RT[0][7] = ctr;
            
        /* for(i=1; i<8; i++) UPDATE_TWEAK(RT, i); */
        for(i=1; i<8; i++){ //UPDATE_TWEAK 
            RT[i][0] = RT[i-1][0];
            RT[i][1] = RT[i-1][1];
            RT[i][2] = RT[i-1][2];
            RT[i][3] = RT[i-1][3];
            RT[i][4] = RT[i-1][4];
            RT[i][5] = RT[i-1][5];
            RT[i][6] = RT[i-1][6];
            RT[i][7] = RT[i-1][7];
        }
        States[0] = ctp[index  ];
        States[1] = ctp[index+1];
        States[2] = ctp[index+2];
        States[3] = ctp[index+3];
        States[4] = ctp[index+4];
        States[5] = ctp[index+5];
        States[6] = ctp[index+6];
        States[7] = ctp[index+7];
        DEOXYS( States, ctx->round_keys_h, RT )   
        
        X = XOR(X, States[0]);
        X = XOR(X, States[1]);
        X = XOR(X, States[2]);
        X = XOR(X, States[3]);
        X = XOR(X, States[4]);
        X = XOR(X, States[5]);
        X = XOR(X, States[6]);
        X = XOR(X, States[7]);
        Y = gf_2_128_double_eight(Y, States);
        /* Y = increment(Y);Y = XOR(Y, States[0]); */  
        /* Y = increment(Y);Y = XOR(Y, States[1]); */  
        /* Y = increment(Y);Y = XOR(Y, States[2]); */  
        /* Y = increment(Y);Y = XOR(Y, States[3]); */  
        /* Y = increment(Y);Y = XOR(Y, States[4]); */  
        /* Y = increment(Y);Y = XOR(Y, States[5]); */  
        /* Y = increment(Y);Y = XOR(Y, States[6]); */  
        /* Y = increment(Y);Y = XOR(Y, States[7]); */  

        index += BPI;
        --iters;
    }

    remaining = local_len % npblks;
    while (remaining >= 16) {
        ctr = ADD_ONE(ctr); T = ctr; S = ctp[index];
        TAES(S, ctx->round_keys_h, T, t);
        X = XOR(X, S);
        Y = increment(Y);Y = XOR(Y, S);  
        index += 1;
        remaining -= 16;
    }
    if (remaining > 0) { //If the last block is not full
        ctr = ADD_ONE(ctr); T = XOR(ctr, ONE); //DomainSep
        S = ZERO();
        memcpy(&S, ctp+index, remaining);
        ((unsigned char *)&S)[remaining] = (unsigned char)0x80; //padding
        TAES(S, ctx->round_keys_h, T, t);
        X = XOR(X, S);
        Y = increment(Y);Y = XOR(Y, S);  
    }
    Y = increment(Y);

/*----------------- Process First Two Blocks ----------------------*/
    //FInalization Of Hash
    Z = X; W = Y;
    Y = OR(Y, TWO); X = OR(X, THREE);
    TAES(Z, ctx->round_keys_h, Y, t);
    TAES(W, ctx->round_keys_h, X, t);

    ctp[0] = XOR(ctp[0], Z);
    ctp[1] = XOR(ctp[1], W);

    return (int) pt_len;
}

