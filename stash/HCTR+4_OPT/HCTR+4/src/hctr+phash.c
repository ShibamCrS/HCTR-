#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

#define BPI 4

#include "../include/setup.h"
#include "../include/deoxysbc.h"
#include "../include/init.h"
/* #define PRINT */
void printregm(const void *a, int nrof_byte){
    int i;
    unsigned char *f = (unsigned char *)a;
    for(i=0; i < nrof_byte; i++){
        printf("%02X",(unsigned char) f[nrof_byte - 1 -i]); //uint8_t c[4+8];
        if(nrof_byte > 16) if((i%16) == 15) printf(" ");
    }
    printf("\n");
}

//multiply by 2 over finite field
static inline BLOCK Double(BLOCK b) {
    const __m128i mask = _mm_set_epi32(135,1,1,1);
    __m128i t = _mm_srai_epi32(b, 31);
    t = _mm_and_si128(t, mask);
    t = _mm_shuffle_epi32(t, _MM_SHUFFLE(2,1,0,3));
    b = _mm_slli_epi32(b, 1);
    return _mm_xor_si128(b,t);
}

/*
    computing:
    y_new = y_old + x[0] + x[1] + x[2] + x[3]
*/
#define accumulate_four_stateful(y, x) { \
    x[0] = XOR(x[0], x[1]); \
    x[2] = XOR(x[2], x[3]); \
    x[3] = XOR(x[0], x[2]); \
    y    = XOR(y, x[3]); \
}

#define REDUCTION_POLYNOMIAL  _mm_set_epi32(0, 0, 0, 135)
#define accumulate_four(x, y) { \
    x[0] = XOR(x[0], x[1]); \
    x[2] = XOR(x[2], x[3]); \
    y    = XOR(x[0], x[2]); \
}
/*
    computing:
    sum = 2(2(2(2Y + S[0]) + S[1]) + S[2]) + S[3]
        = 2^4Y + 2^3S[0] + 2^2S[1] + 2S[2] + S[3]
*/
__m128i gf_2_128_double_four(__m128i Y, __m128i S[4]) {
    __m128i tmp[4];
    tmp[0] = _mm_srli_epi64(Y   , 60);
    tmp[1] = _mm_srli_epi64(S[0], 61);
    tmp[2] = _mm_srli_epi64(S[1], 62);
    tmp[3] = _mm_srli_epi64(S[2], 63);

    __m128i sum;
    accumulate_four(tmp, sum);

    __m128i mod =  _mm_clmulepi64_si128(sum, REDUCTION_POLYNOMIAL, 0x01);

    __m128i sum_low = _mm_bslli_si128(sum, 8);

    tmp[0] = _mm_slli_epi64(Y,    4);
    tmp[1] = _mm_slli_epi64(S[0], 3);
    tmp[2] = _mm_slli_epi64(S[1], 2);
    tmp[3] = _mm_slli_epi64(S[2], 1);

    accumulate_four(tmp, sum);
    sum = XOR(sum, sum_low);
    sum = XOR(sum, mod);
    sum = XOR(sum, S[3]);
    return sum;
}

int prp_encrypt(prp_ctx     * restrict ctx,
               const void *pt,
               uint64_t    pt_len,
               const void *tk,
               uint64_t    tk_len,
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

    uint64_t len1 = pt_len*8;
    uint64_t len2 = tk_len*8;
    BLOCK LEN = _mm_set_epi64x(len1, len2);

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
            
        /* for(i=1; i<8; i++) UPDATE_TWEAK(RT, i); */
        for(i=1; i<8; i++){ //UPDATE_TWEAK 
            RT[i][0] = PERMUTE(RT[i-1][0]);
            RT[i][1] = PERMUTE(RT[i-1][1]);
            RT[i][2] = PERMUTE(RT[i-1][2]);
            RT[i][3] = PERMUTE(RT[i-1][3]);
        }
        States[0] = tkp[index  ];
        States[1] = tkp[index+1];
        States[2] = tkp[index+2];
        States[3] = tkp[index+3];
        DEOXYS( States, ctx->round_keys_h, RT )   
        
        
        Y = gf_2_128_double_four(Y, States);
        accumulate_four_stateful(X, States);

        index += BPI;
        --iters;
    }
    
    remaining = local_len % npblks;
    while (remaining >= 16) {
        ctr = ADD_ONE(ctr); T = ctr; S = tkp[index];
        TAES(S, ctx->round_keys_h, T, t);
        X = XOR(X, S);
        Y = Double(Y); Y = XOR(Y, S);   
        index += 1;
        remaining -= 16;
    }
    if (remaining > 0) { //If the last block is not full
        ctr = ADD_ONE(ctr); T = ctr; //DomainSep
        S = ZERO();
        memcpy(&S, tkp+index, remaining); //With 0* padding
        TAES(S, ctx->round_keys_h, T, t);
        X = XOR(X, S);
        Y = Double(Y); Y = XOR(Y, S);   
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
        
        for(i=1; i<8; i++){ //UPDATE_TWEAK 
            RT[i][0] = PERMUTE(RT[i-1][0]);
            RT[i][1] = PERMUTE(RT[i-1][1]);
            RT[i][2] = PERMUTE(RT[i-1][2]);
            RT[i][3] = PERMUTE(RT[i-1][3]);
        }
        States[0] = ptp[index  ];
        States[1] = ptp[index+1];
        States[2] = ptp[index+2];
        States[3] = ptp[index+3];
        DEOXYS( States, ctx->round_keys_h, RT )   
        
        Y = gf_2_128_double_four(Y, States);
        accumulate_four_stateful(X, States);

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
        Y = Double(Y); Y = XOR(Y, S);   
        index += 1;
        remaining -= 16;
    }
    if (remaining > 0) { //If the last block is not full
        ctr = ADD_ONE(ctr); T = ctr;
        S = ZERO();
        memcpy(&S, ptp+index, remaining);
        TAES(S, ctx->round_keys_h, T, t);
        X = XOR(X, S);
        Y = Double(Y); Y = XOR(Y, S);   
    }
    ctr = ADD_ONE(ctr);
    T = XOR(ctr, ONE);
    
    //Handel Length 
    S = LEN;
    TAES(S, ctx->round_keys_h, T, t);
    X = XOR(X, S);
    Y = Double(Y); Y = XOR(Y, S);  
    //FInalization Of Hash
    Z = X; W = Y;
    Y = TRUNC(Y, TWO); X = TRUNC(X, THREE);
    TAES(Z, ctx->round_keys_h, Y, t);
    TAES(W, ctx->round_keys_h, X, t);
/*-----------------Process Upper Part Of The First Two Blocks----------------*/
    Z = XOR(ptp[0], Z);
    W = XOR(ptp[1], W);
    if(encrypt) {
#ifdef PRINT
    printf("U1: "); printregm(&Z, 16);
    printf("U2: "); printregm(&W, 16);
#endif
    TAES(Z, ctx->round_keys_1, W, t);
    TAES(W, ctx->round_keys_2, Z, t);
#ifdef PRINT
    printf("Z : "); printregm(&Z, 16);
    printf("W : "); printregm(&W, 16);
#endif
    S = Z; T = W;
    TAES(S, ctx->round_keys_3, T, t);
    TAES(T, ctx->round_keys_4, S, t);
#ifdef PRINT
    printf("V1: "); printregm(&S, 16);
    printf("V2: "); printregm(&T, 16);
#endif
    }
    else {
#ifdef PRINT
    printf("V1: "); printregm(&Z, 16);
    printf("V2: "); printregm(&W, 16);
#endif
    TAESD(W, ctx->round_keys_d_4, Z, t);
    TAESD(Z, ctx->round_keys_d_3, W, t);
#ifdef PRINT
    printf("Z : "); printregm(&Z, 16);
    printf("W : "); printregm(&W, 16);
#endif
    S = Z; T = W;
    TAESD(T, ctx->round_keys_d_2, S, t);
    TAESD(S, ctx->round_keys_d_1, T, t);
#ifdef PRINT
    printf("U1: "); printregm(&S, 16);
    printf("U2: "); printregm(&T, 16);
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
            
        for(i=1; i<8; i++){ //UPDATE_TWEAK 
            RT[i][0] = PERMUTE(RT[i-1][0]);
            RT[i][1] = PERMUTE(RT[i-1][1]);
            RT[i][2] = PERMUTE(RT[i-1][2]);
            RT[i][3] = PERMUTE(RT[i-1][3]);
        }
        States[0] = States[1] = States[2] = States[3] = W;
        DEOXYS( States, ctx->round_keys_c, RT )   
        
        ctp[index    ] = XOR(States[0], ptp[index    ]);
        ctp[index + 1] = XOR(States[1], ptp[index + 1]);
        ctp[index + 2] = XOR(States[2], ptp[index + 2]);
        ctp[index + 3] = XOR(States[3], ptp[index + 3]);
        
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
        
        for(i=1; i<8; i++){ //UPDATE_TWEAK 
            RT[i][0] = PERMUTE(RT[i-1][0]);
            RT[i][1] = PERMUTE(RT[i-1][1]);
            RT[i][2] = PERMUTE(RT[i-1][2]);
            RT[i][3] = PERMUTE(RT[i-1][3]);
        }    
        States[0] = ctp[index  ];
        States[1] = ctp[index+1];
        States[2] = ctp[index+2];
        States[3] = ctp[index+3];
        DEOXYS( States, ctx->round_keys_h, RT )   
        
        Y = gf_2_128_double_four(Y, States);
        accumulate_four_stateful(X, States);

        index += BPI;
        --iters;
    }
    remaining = local_len % npblks;
    while (remaining >= 16) {
        ctr = ADD_ONE(ctr); T = ctr; S = ctp[index];
        TAES(S, ctx->round_keys_h, T, t);
        X = XOR(X, S);
        Y = Double(Y); Y = XOR(Y, S);  
        index += 1;
        remaining -= 16;
    }
    if (remaining > 0) { //If the last block is not full
        ctr = ADD_ONE(ctr); T = ctr; //DomainSep
        S = ZERO();
        memcpy(&S, ctp+index, remaining);
        TAES(S, ctx->round_keys_h, T, t);
        X = XOR(X, S);
        Y = Double(Y); Y = XOR(Y, S);  
    }
    //Handel Length 
    ctr = ADD_ONE(ctr);
    T = XOR(ctr, ONE);
    S = LEN;
    TAES(S, ctx->round_keys_h, T, t);
    X = XOR(X, S);
    Y = Double(Y); Y = XOR(Y, S);  
/*----------------- Process First Two Blocks ----------------------*/
    //FInalization Of Hash
    Z = X; W = Y;
    Y = TRUNC(Y, TWO); X = TRUNC(X, THREE);
    TAES(Z, ctx->round_keys_h, Y, t);
    TAES(W, ctx->round_keys_h, X, t);

    ctp[0] = XOR(ctp[0], Z);
    ctp[1] = XOR(ctp[1], W);

    return (int) pt_len;
}

