#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

#define BPI 4
/* #define PRINT */
#include "../include/setup.h"
#include "../include/deoxysbc.h"
#include "../include/init.h"

void printregz(const void *a, int nrof_byte){
    int i;
    unsigned char *f = (unsigned char *)a;
    for(i=0; i < nrof_byte; i++){
        printf("%02X",(unsigned char) f[nrof_byte - 1 -i]); //uint8_t c[4+8];
        if(nrof_byte > 16) if((i%16) == 15) printf(" ");
    }
    printf("\n");
}

static inline BLOCK increment(BLOCK b) {
    const __m128i mask = _mm_set_epi32(135,1,1,1);
    __m128i t = _mm_srai_epi32(b, 31);
    t = _mm_and_si128(t, mask);
    t = _mm_shuffle_epi32(t, _MM_SHUFFLE(2,1,0,3));
    b = _mm_slli_epi32(b, 1);
    return _mm_xor_si128(b,t);
}
#define REDUCTION_POLYNOMIAL  _mm_set_epi32(0, 0, 0, 135)
#define accumulate_four(x, y) { \
    x[0] = XOR(x[0], x[1]); \
    x[2] = XOR(x[2], x[3]); \
    y    = XOR(x[0], x[2]); \
}
#define accumulate_four_stateful(x, y) { \
    x[0] = XOR(x[0], x[1]); \
    x[2] = XOR(x[2], x[3]); \
    x[3] = XOR(x[0], x[2]); \
    y    = XOR(y, x[3]); \
}
static inline BLOCK gf_2_128_double_four(BLOCK Y, BLOCK S[4]) {
    BLOCK tmp[4];
    tmp[0] = _mm_srli_epi64(Y   , 60);
    tmp[1] = _mm_srli_epi64(S[0], 61);
    tmp[2] = _mm_srli_epi64(S[1], 62);
    tmp[3] = _mm_srli_epi64(S[2], 63);

    BLOCK sum;
    accumulate_four(tmp, sum);

    // ---------------------------------------------------------------------
    // sum = sum_high || sum_low
    // We have to take sum_high * 135 and XOR it to our XOR sum to have the
    // Reduction term. The 0x01 indicates that sum_high is used.
    // ---------------------------------------------------------------------

    BLOCK mod =  _mm_clmulepi64_si128(sum, REDUCTION_POLYNOMIAL, 0x01);

    // Move sum_low to the upper 64-bit half
    BLOCK sum_low = _mm_bslli_si128(sum, 8);

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
    unsigned i, j, k, remaining=0, iters, npblks, index, local_len, index4;
    BLOCK       * restrict ctp = (BLOCK *)ct;
    const BLOCK * restrict ptp = (BLOCK *)pt;
    const BLOCK * restrict tkp = (BLOCK *)tk;

    const BLOCK4 * restrict ptp4 = (BLOCK4 *)(ptp + 2);
          BLOCK4 * restrict ctp4 = (BLOCK4 *)(ctp + 2);

    BLOCK X, Y, Z, W, ctr;
    BLOCK T, t, S;
    BLOCK ST[2], RT[8];
    BLOCK States[BPI];
    npblks = 16*BPI;
    
    BLOCK4 RT4[8];
    BLOCK4 state, ctr4;

    
    __m512i round_keys_h[DEOXYS_BC_128_256_NUM_ROUND_KEYS];
    __m512i round_keys_c[DEOXYS_BC_128_256_NUM_ROUND_KEYS];
    for(int i=0; i<=DEOXYS_BC_128_256_NUM_ROUNDS; i++){
        round_keys_h[i] = _mm512_broadcast_i32x4(ctx->round_keys_h[i]);
        round_keys_c[i] = _mm512_broadcast_i32x4(ctx->round_keys_c[i]);
    }
    
    //Setup Ll and Lr
    __m128i Ll = ZERO(); __m128i Lr = ZERO();
    T = ZERO();
    TAES(Ll, ctx->round_keys_h, T, t);
    T = ADD_ONE(T);
    TAES(Lr, ctx->round_keys_h, T, t);
    BLOCK XX[4];

/* --------------- The Upper Hash using PMAC2x -------------------------*/
/* --------------------------------- Process Tweaks -------------------------*/
    local_len = tk_len;
    index = 0;
    iters = local_len/(npblks*2);
    X = ZERO();
    Y = ZERO();
    while (iters) {
        States[0] = XOR(Ll, tkp[index     ]); Ll = increment(Ll);
        States[1] = XOR(Ll, tkp[index +  2]); Ll = increment(Ll);
        States[2] = XOR(Ll, tkp[index +  4]); Ll = increment(Ll);
        States[3] = XOR(Ll, tkp[index +  6]); Ll = increment(Ll);

        RT[0]  = XOR(Lr, tkp[index +  1]); Lr = increment(Lr); 
        RT[1]  = XOR(Lr, tkp[index +  3]); Lr = increment(Lr); 
        RT[2]  = XOR(Lr, tkp[index +  5]); Lr = increment(Lr); 
        RT[3]  = XOR(Lr, tkp[index +  7]); Lr = increment(Lr); 
        
        state = *((BLOCK4 *)States);
        RT4[0] = *((BLOCK4 *)RT);
        RT4[0] = TRUNC4(RT4[0], ONE4);
        for(i=1; i<8; i++){ //UPDATE_TWEAK 
            RT4[i] = PERMUTE_512(RT4[i-1]);
        }
        DEOXYS( state, round_keys_h, RT4 )
        
        States[0] = _mm512_extracti32x4_epi32(state, 0);
        States[1] = _mm512_extracti32x4_epi32(state, 1);
        States[2] = _mm512_extracti32x4_epi32(state, 2);
        States[3] = _mm512_extracti32x4_epi32(state, 3);

        XX[0] = XOR(States[0], tkp[index + 1]); \
        XX[1] = XOR(States[1], tkp[index + 3]); \
        XX[2] = XOR(States[2], tkp[index + 5]); \
        XX[3] = XOR(States[3], tkp[index + 7]);
        
        accumulate_four_stateful(XX, X);
        Y = gf_2_128_double_four(Y, States);

        index += 8;
        --iters;
    }

    remaining = local_len % (npblks*2);
    while (remaining >= 32) {
        S = XOR(Ll, tkp[index]);      Ll = increment(Ll); 
        T = XOR(Lr, tkp[index +  1]); Lr = increment(Lr); 
        TAES(S, ctx->round_keys_h, T, t);
        X = XOR(XOR(X, S), tkp[index+1]);
        Y = XOR(Y, S); Y = increment(Y); 
        index += 2;
        remaining -= 32;
    }
    if (remaining > 0) { //If the second last block is full and the last block is not full
        ST[0] = ZERO();
        ST[1] = ZERO();
        memcpy(ST, tkp+index, remaining);
        S = XOR(Ll, ST[0]); Ll = increment(Ll); 
        T = XOR(Lr, ST[1]); Lr = increment(Lr);
        T = XOR(T, ONE); //DomainSep
        TAES(S, ctx->round_keys_h, T, t);

        X = XOR(XOR(X, S), ST[1]);
        Y = XOR(Y, S); Y = increment(Y); 
    }
    BLOCK HT0 = X;
    BLOCK HT1 = Y;
    BLOCK Ll_SAVE = Ll;
    BLOCK Lr_SAVE = Lr;
/*-----------------------Process Plaintexts----------------------------*/
    local_len = (pt_len - 2*16); //First two message block will be used later
    index = 2; //First two message block will be used later
    iters = local_len/(2*npblks);
    while (iters) {
        States[0] = XOR(Ll, ptp[index     ]); Ll = increment(Ll); 
        States[1] = XOR(Ll, ptp[index +  2]); Ll = increment(Ll); 
        States[2] = XOR(Ll, ptp[index +  4]); Ll = increment(Ll); 
        States[3] = XOR(Ll, ptp[index +  6]); Ll = increment(Ll); 

        RT[0]  = XOR(Lr, ptp[index +  1]); Lr = increment(Lr); 
        RT[1]  = XOR(Lr, ptp[index +  3]); Lr = increment(Lr); 
        RT[2]  = XOR(Lr, ptp[index +  5]); Lr = increment(Lr); 
        RT[3]  = XOR(Lr, ptp[index +  7]); Lr = increment(Lr); 

        state = *((BLOCK4 *)States);
        RT4[0] = *((BLOCK4 *)RT);
        RT4[0] = TRUNC4(RT4[0], ONE4);
        for(i=1; i<8; i++){ //UPDATE_TWEAK 
            RT4[i] = PERMUTE_512(RT4[i-1]);
        }
        DEOXYS( state, round_keys_h, RT4 )

        States[0] = _mm512_extracti32x4_epi32(state, 0);
        States[1] = _mm512_extracti32x4_epi32(state, 1);
        States[2] = _mm512_extracti32x4_epi32(state, 2);
        States[3] = _mm512_extracti32x4_epi32(state, 3);

        XX[0] = XOR(States[0], ptp[index + 1]); \
        XX[1] = XOR(States[1], ptp[index + 3]); \
        XX[2] = XOR(States[2], ptp[index + 5]); \
        XX[3] = XOR(States[3], ptp[index + 7]);
        
        accumulate_four_stateful(XX, X);
        Y = gf_2_128_double_four(Y, States);

        index += 8;
        --iters;
    }

    remaining = local_len % (npblks*2);
    while (remaining >= 32) {
        S = XOR(Ll, ptp[index]);      Ll = increment(Ll); 
        T = XOR(Lr, ptp[index +  1]); Lr = increment(Lr); 
        T = XOR(TWO, T);
        TAES(S, ctx->round_keys_h, T, t);
        X = XOR(XOR(X, S), ptp[index+1]);
        Y = increment(Y);Y = XOR(Y, S);  
        index += 2;
        remaining -= 32;
    }
    if (remaining > 0) { //If the second last block is full and the last block is not full
        ST[0] = ZERO();
        ST[1] = ZERO();
        memcpy(ST, ptp+index, remaining);
        S = XOR(Ll, ST[0]); Ll = increment(Ll); 
        T = XOR(Lr, ST[1]); Lr = increment(Lr);
        T = XOR(T, THREE); //DomainSep
        TAES(S, ctx->round_keys_h, T, t);

        X = XOR(XOR(X, S), ST[1]);
        Y = increment(Y);Y = XOR(Y, S);  
    }
    //Handel Length 
    uint64_t len1 = pt_len*8; 
    uint64_t len2 = tk_len*8; 
    S = _mm_set_epi64x(len1, len2);
    S = XOR(Ll, S);
    TAES(S, ctx->round_keys_h, Lr, t);
    X = XOR(X, S);
    Y = increment(Y);Y = XOR(Y, S);  
    Y = increment(Y);
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
    printf("U1: "); printregz(&Z, 16);
    printf("U2: "); printregz(&W, 16);
#endif
    TAES(Z, ctx->round_keys_1, W, t);
    TAES(W, ctx->round_keys_2, Z, t);
#ifdef PRINT
    printf("Z : "); printregz(&Z, 16);
    printf("W : "); printregz(&W, 16);
#endif
    S = Z; T = W;
    TAES(S, ctx->round_keys_3, T, t);
    TAES(T, ctx->round_keys_4, S, t);
#ifdef PRINT
    printf("V1: "); printregz(&S, 16);
    printf("V2: "); printregz(&T, 16);
#endif
    }
    else {
#ifdef PRINT
    printf("V1: "); printregz(&Z, 16);
    printf("V2: "); printregz(&W, 16);
#endif
    TAESD(W, ctx->round_keys_d_4, Z, t);
    TAESD(Z, ctx->round_keys_d_3, W, t);
#ifdef PRINT
    printf("Z : "); printregz(&Z, 16);
    printf("W : "); printregz(&W, 16);
#endif
    S = Z; T = W;
    TAESD(T, ctx->round_keys_d_2, S, t);
    TAESD(S, ctx->round_keys_d_1, T, t);
#ifdef PRINT
    printf("U1: "); printregz(&S, 16);
    printf("U2: "); printregz(&T, 16);
#endif
    }
    ctp[0] = S; ctp[1] = T;
/*------------------------------------ The CTR Part -------------------------*/
    local_len = (pt_len - 2*16); //First two message block will be used later
    iters = local_len/npblks;
    index  = 2; //First two message block will be used later
    index4 = 0;
    ctr4 = CTR4321;
    __m512i WW = _mm512_broadcast_i32x4(W);
    __m512i ZZ = _mm512_broadcast_i32x4(Z);
    __m512i ZC = XOR4(ZZ, ctr4);
    while (iters) {
        RT4[0]  = ZC;
        ctr4   =  ADD4444(ctr4);
        ZC = XOR4(ZZ, ctr4);
        for(i=1; i<8; i++){ //UPDATE_TWEAK 
            RT4[i] = PERMUTE_512(RT4[i-1]);
        }
        state = WW;
        DEOXYS( state, round_keys_c, RT4 )
        ctp4[index4] = XOR4(state, ptp4[index4]);

        index  += BPI;
        index4 += 1;
        --iters;
    }
    ctr = _mm512_extracti32x4_epi32(ctr4, 0);
    remaining = local_len % npblks;
    while (remaining >= 16) {
        T = XOR(ctr, Z); ctr = ADD_ONE(ctr);
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
    iters = local_len/(2*npblks);
    X = HT0;
    Y = HT1;
    Ll = Ll_SAVE;
    Lr = Lr_SAVE;
    while (iters) {
        States[0] = XOR(Ll, ctp[index     ]); Ll = increment(Ll); 
        States[1] = XOR(Ll, ctp[index +  2]); Ll = increment(Ll); 
        States[2] = XOR(Ll, ctp[index +  4]); Ll = increment(Ll); 
        States[3] = XOR(Ll, ctp[index +  6]); Ll = increment(Ll); 

        RT[0]  = XOR(Lr, ctp[index +  1]); Lr = increment(Lr); 
        RT[1]  = XOR(Lr, ctp[index +  3]); Lr = increment(Lr); 
        RT[2]  = XOR(Lr, ctp[index +  5]); Lr = increment(Lr); 
        RT[3]  = XOR(Lr, ctp[index +  7]); Lr = increment(Lr); 

        state = *((BLOCK4 *)States);
        RT4[0] = *((BLOCK4 *)RT);
        RT4[0] = TRUNC4(RT4[0], ONE4);
        for(i=1; i<8; i++){ //UPDATE_TWEAK 
            RT4[i] = PERMUTE_512(RT4[i-1]);
        }
        DEOXYS( state, round_keys_h, RT4 )

        States[0] = _mm512_extracti32x4_epi32(state, 0);
        States[1] = _mm512_extracti32x4_epi32(state, 1);
        States[2] = _mm512_extracti32x4_epi32(state, 2);
        States[3] = _mm512_extracti32x4_epi32(state, 3);

        XX[0] = XOR(States[0], ctp[index + 1]); \
        XX[1] = XOR(States[1], ctp[index + 3]); \
        XX[2] = XOR(States[2], ctp[index + 5]); \
        XX[3] = XOR(States[3], ctp[index + 7]);
        
        accumulate_four_stateful(XX, X);
        Y = gf_2_128_double_four(Y, States);
        
        index += 8;
        --iters;
    }

    remaining = local_len % (npblks*2);
    while (remaining >= 32) {
        S = XOR(Ll, ctp[index]);      Ll = increment(Ll); 
        T = XOR(Lr, ctp[index +  1]); Lr = increment(Lr); 
        T = XOR(TWO, T);
        TAES(S, ctx->round_keys_h, T, t);
        X = XOR(XOR(X, S), ctp[index+1]);
        Y = increment(Y);Y = XOR(Y, S);  
        index += 2;
        remaining -= 32;
    }
    if (remaining > 0) { //If the second last block is full and the last block is not full
        ST[0] = ZERO();
        ST[1] = ZERO();
        memcpy(ST, ctp+index, remaining);
        ((unsigned char *)ST)[remaining] = (unsigned char)0x80; //padding
        
        S = XOR(Ll, ST[0]); Ll = increment(Ll); 
        T = XOR(Lr, ST[1]); Lr = increment(Lr);
        T = XOR(T, THREE); //DomainSep
        TAES(S, ctx->round_keys_h, T, t);

        X = XOR(XOR(X, S), ST[1]);
        Y = increment(Y);Y = XOR(Y, S);  
    }
    //Handel Length 
    len1 = pt_len*8; 
    len2 = tk_len*8; 
    S = _mm_set_epi64x(len1, len2);

    S = XOR(Ll, S);
    TAES(S, ctx->round_keys_h, Lr, t);
    X = XOR(X, S);
    Y = increment(Y);Y = XOR(Y, S);  
    Y = increment(Y);
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
