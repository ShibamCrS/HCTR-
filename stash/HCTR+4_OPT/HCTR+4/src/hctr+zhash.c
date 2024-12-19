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

void printregm(const void *a, int nrof_byte){
    int i;
    unsigned char *f = (unsigned char *)a;
    for(i=0; i < nrof_byte; i++){
        printf("%02X",(unsigned char) f[nrof_byte - 1 -i]); //uint8_t c[4+8];
    }
    printf("\n");
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
    BLOCK U, V, Z, W, ctr;
    BLOCK T, t, S;
    BLOCK ST[2];
    npblks = 16*BPI;
    
    uint64_t len1 = pt_len*8; 
    uint64_t len2 = tk_len*8; 
    BLOCK LEN = _mm_set_epi64x(len1, len2);

    BLOCK SR[8][BPI];
    BLOCK SL[BPI];

    //Setup Ll and Lr
    __m128i Ll = ZERO(); __m128i Lr = ZERO();
    T = ZERO();
    TAES(Ll, ctx->round_keys_h, T, t);
    T = ADD_ONE(T);
    TAES(Lr, ctx->round_keys_h, T, t);
    BLOCK CR[4];

/* --------------- The Upper Hash using PMAC2x -------------------------*/
/* --------------------------------- Process Tweaks -------------------------*/
    local_len = tk_len;
    index = 0;
    iters = local_len/(npblks*2);
    U = ZERO();
    V = ZERO();
    while (iters) {
        SL[0] = XOR(Ll, tkp[index     ]); Ll = Double(Ll); 
        SL[1] = XOR(Ll, tkp[index +  2]); Ll = Double(Ll); 
        SL[2] = XOR(Ll, tkp[index +  4]); Ll = Double(Ll); 
        SL[3] = XOR(Ll, tkp[index +  6]); Ll = Double(Ll); 

        SR[0][0]  = XOR(Lr, tkp[index +  1]); Lr = Double(Lr); 
        SR[0][1]  = XOR(Lr, tkp[index +  3]); Lr = Double(Lr); 
        SR[0][2]  = XOR(Lr, tkp[index +  5]); Lr = Double(Lr); 
        SR[0][3]  = XOR(Lr, tkp[index +  7]); Lr = Double(Lr); 
        
        SR[0][0] = TRUNC(SR[0][0],ONE);\
        SR[0][1] = TRUNC(SR[0][1],ONE);\
        SR[0][2] = TRUNC(SR[0][2],ONE);\
        SR[0][3] = TRUNC(SR[0][3],ONE);
        
        for(i=1; i<8; i++){ //UPDATE_TWEAK 
            SR[i][0] = PERMUTE(SR[i-1][0]);
            SR[i][1] = PERMUTE(SR[i-1][1]);
            SR[i][2] = PERMUTE(SR[i-1][2]);
            SR[i][3] = PERMUTE(SR[i-1][3]);
        }
        DEOXYS( SL, ctx->round_keys_h, SR )  //CL == SL

        U = gf_2_128_double_four(U, SL);
        
        SL[0] = XOR(SL[0], tkp[index + 1]); \
        SL[1] = XOR(SL[1], tkp[index + 3]); \
        SL[2] = XOR(SL[2], tkp[index + 5]); \
        SL[3] = XOR(SL[3], tkp[index + 7]);
        
        accumulate_four_stateful(V, SL);
                
        index += 8;
        --iters;
    }

    remaining = local_len % (npblks*2);
    while (remaining >= 32) {
        S = XOR(Ll, tkp[index]);      Ll = Double(Ll); 
        T = XOR(Lr, tkp[index +  1]); Lr = Double(Lr); 
        TAES(S, ctx->round_keys_h, T, t);
        U = Double(U); U = XOR(U, S);  
        V = XOR(V, XOR(S, tkp[index+1]));
        index += 2;
        remaining -= 32;
    }
    if (remaining > 0) {
        ST[0] = ZERO();
        ST[1] = ZERO();
        memcpy(ST, tkp+index, remaining);
        S = XOR(Ll, ST[0]); Ll = Double(Ll); 
        T = XOR(Lr, ST[1]); Lr = Double(Lr);
        T = XOR(T, ONE); //DomainSep
        TAES(S, ctx->round_keys_h, T, t);

        U = Double(U); U = XOR(U, S);  
        V = XOR(V,  XOR(S, ST[1]));
    }
    BLOCK HT0 = U;
    BLOCK HT1 = V;
    BLOCK Ll_SAVE = Ll;
    BLOCK Lr_SAVE = Lr;
/*-----------------------Process Plaintexts----------------------------*/
    local_len = (pt_len - 2*16); //First two message block will be used later
    index = 2; //First two message block will be used later
    iters = local_len/(2*npblks);
    while (iters) {
        SL[0] = XOR(Ll, ptp[index     ]); Ll = Double(Ll); 
        SL[1] = XOR(Ll, ptp[index +  2]); Ll = Double(Ll); 
        SL[2] = XOR(Ll, ptp[index +  4]); Ll = Double(Ll); 
        SL[3] = XOR(Ll, ptp[index +  6]); Ll = Double(Ll); 

        SR[0][0]  = XOR(Lr, ptp[index +  1]); Lr = Double(Lr); 
        SR[0][1]  = XOR(Lr, ptp[index +  3]); Lr = Double(Lr); 
        SR[0][2]  = XOR(Lr, ptp[index +  5]); Lr = Double(Lr); 
        SR[0][3]  = XOR(Lr, ptp[index +  7]); Lr = Double(Lr); 

        SR[0][0] = TRUNC(SR[0][0],ONE);\
        SR[0][1] = TRUNC(SR[0][1],ONE);\
        SR[0][2] = TRUNC(SR[0][2],ONE);\
        SR[0][3] = TRUNC(SR[0][3],ONE);

        for(i=1; i<8; i++){ //UPDATE_TWEAK 
            SR[i][0] = PERMUTE(SR[i-1][0]);
            SR[i][1] = PERMUTE(SR[i-1][1]);
            SR[i][2] = PERMUTE(SR[i-1][2]);
            SR[i][3] = PERMUTE(SR[i-1][3]);
        }
        DEOXYS( SL, ctx->round_keys_h, SR )  //CL == SL
        
        U = gf_2_128_double_four(U, SL);
        
        SL[0] = XOR(SL[0], tkp[index + 1]); \
        SL[1] = XOR(SL[1], tkp[index + 3]); \
        SL[2] = XOR(SL[2], tkp[index + 5]); \
        SL[3] = XOR(SL[3], tkp[index + 7]);
        
        accumulate_four_stateful(V, SL);

        index += 8;
        --iters;
    }

    remaining = local_len % (npblks*2);
    while (remaining >= 32) {
        S = XOR(Ll, ptp[index]);      Ll = Double(Ll); 
        T = XOR(Lr, ptp[index +  1]); Lr = Double(Lr); 
        T = XOR(TWO, T);
        TAES(S, ctx->round_keys_h, T, t);
        
        U = Double(U); U = XOR(U, S);  
        V = XOR(V, XOR(S, ptp[index+1]));
        index += 2;
        remaining -= 32;
    }
    if (remaining > 0) { //If the second last block is full and the last block is not full
        ST[0] = ZERO();
        ST[1] = ZERO();
        memcpy(ST, ptp+index, remaining);
        S = XOR(Ll, ST[0]); Ll = Double(Ll); 
        T = XOR(Lr, ST[1]); Lr = Double(Lr);
        T = XOR(T, THREE); //DomainSep
        TAES(S, ctx->round_keys_h, T, t);
        
        U = Double(U); U = XOR(U, S);  
        V = XOR(V,  XOR(S, ST[1]));
    }
    //Handel Length 
    S = XOR(Ll, LEN);
    TAES(S, ctx->round_keys_h, Lr, t);
    U = Double(U); U = XOR(U, S); 
    V = XOR(V, S);
    //FInalization Of Hash
    Z = U; W = U;
    U = TRUNC(V, TWO); V = TRUNC(V, THREE);
    TAES(Z, ctx->round_keys_h, U, t);
    TAES(W, ctx->round_keys_h, V, t);

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
        ctr =  ADD_ONE(ctr); SR[0][0] = XOR(ctr, Z);
        ctr =  ADD_ONE(ctr); SR[0][1] = XOR(ctr, Z);
        ctr =  ADD_ONE(ctr); SR[0][2] = XOR(ctr, Z);
        ctr =  ADD_ONE(ctr); SR[0][3] = XOR(ctr, Z);
            
        for(i=1; i<8; i++){ //UPDATE_TWEAK
            SR[i][0] = PERMUTE(SR[i-1][0]);
            SR[i][1] = PERMUTE(SR[i-1][1]);
            SR[i][2] = PERMUTE(SR[i-1][2]);
            SR[i][3] = PERMUTE(SR[i-1][3]);
        }
        SL[0] = SL[1] = SL[2] = SL[3] = W;
        DEOXYS( SL, ctx->round_keys_c, SR )   
        
        ctp[index    ] = XOR(SL[0], ptp[index    ]);\
        ctp[index + 1] = XOR(SL[1], ptp[index + 1]);\
        ctp[index + 2] = XOR(SL[2], ptp[index + 2]);\
        ctp[index + 3] = XOR(SL[3], ptp[index + 3]);
        
        index += 4;
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
    iters = local_len/(2*npblks);
    U = HT0;
    V = HT1;
    Ll = Ll_SAVE;
    Lr = Lr_SAVE;
    while (iters) {
        SL[0] = XOR(Ll, ctp[index     ]); Ll = Double(Ll); 
        SL[1] = XOR(Ll, ctp[index +  2]); Ll = Double(Ll); 
        SL[2] = XOR(Ll, ctp[index +  4]); Ll = Double(Ll); 
        SL[3] = XOR(Ll, ctp[index +  6]); Ll = Double(Ll); 

        SR[0][0]  = XOR(Lr, ctp[index +  1]); Lr = Double(Lr); 
        SR[0][1]  = XOR(Lr, ctp[index +  3]); Lr = Double(Lr); 
        SR[0][2]  = XOR(Lr, ctp[index +  5]); Lr = Double(Lr); 
        SR[0][3]  = XOR(Lr, ctp[index +  7]); Lr = Double(Lr); 

        SR[0][0] = TRUNC(SR[0][0],ONE);\
        SR[0][1] = TRUNC(SR[0][1],ONE);\
        SR[0][2] = TRUNC(SR[0][2],ONE);\
        SR[0][3] = TRUNC(SR[0][3],ONE);

        for(i=1; i<8; i++){ //UPDATE_TWEAK 
            SR[i][0] = PERMUTE(SR[i-1][0]);
            SR[i][1] = PERMUTE(SR[i-1][1]);
            SR[i][2] = PERMUTE(SR[i-1][2]);
            SR[i][3] = PERMUTE(SR[i-1][3]);
        }
        DEOXYS( SL, ctx->round_keys_h, SR )  //CL == SL
        
        U = gf_2_128_double_four(U, SL);
        
        SL[0] = XOR(SL[0], tkp[index + 1]); \
        SL[1] = XOR(SL[1], tkp[index + 3]); \
        SL[2] = XOR(SL[2], tkp[index + 5]); \
        SL[3] = XOR(SL[3], tkp[index + 7]);
        
        accumulate_four_stateful(V, SL);

        index += 8;
        --iters;
    }

    remaining = local_len % (npblks*2);
    while (remaining >= 32) {
        S = XOR(Ll, ctp[index]);      Ll = Double(Ll); 
        T = XOR(Lr, ctp[index +  1]); Lr = Double(Lr); 
        T = XOR(TWO, T);
        TAES(S, ctx->round_keys_h, T, t);
        
        U = Double(U); U = XOR(U, S);  
        V = XOR(V, XOR(S, ctp[index+1]));
        index += 2;
        remaining -= 32;
    }
    if (remaining > 0) { //If the second last block is full and the last block is not full
        ST[0] = ZERO();
        ST[1] = ZERO();
        memcpy(ST, ctp+index, remaining);
        S = XOR(Ll, ST[0]); Ll = Double(Ll); 
        T = XOR(Lr, ST[1]); Lr = Double(Lr);
        T = XOR(T, THREE); //DomainSep
        TAES(S, ctx->round_keys_h, T, t);
        
        U = Double(U); U = XOR(U, S);  
        V = XOR(V,  XOR(S, ST[1]));
    }
    //Handel Length 
    S = XOR(Ll, LEN);
    TAES(S, ctx->round_keys_h, Lr, t);
    U = Double(U); U = XOR(U, S); 
    V = XOR(V, S);
/*----------------- Process First Two Blocks ----------------------*/
    //FInalization Of Hash
    Z = U; W = U;
    U = TRUNC(V, TWO); V = TRUNC(V, THREE);
    TAES(Z, ctx->round_keys_h, U, t);
    TAES(W, ctx->round_keys_h, V, t);

    ctp[0] = XOR(ctp[0], Z);
    ctp[1] = XOR(ctp[1], W);

    return (int) pt_len;
}
