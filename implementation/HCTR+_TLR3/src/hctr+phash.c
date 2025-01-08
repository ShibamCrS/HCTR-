#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

#include "../include/setup.h"
#include "../include/deoxysbc.h"
#include "../include/init.h"
#include "../include/utility.h"
/* #define PRINT */


BLOCK phash(const BLOCK * data, const BLOCK key[DEOXYS_BC_128_256_NUM_ROUND_KEYS], uint64_t len, BLOCK ctr, BLOCK *X, BLOCK *Y) {

    unsigned i, remaining, iters, npbytes, index;
    index = 0;
    npbytes = 16*BPI;
    iters = len/npbytes;
    BLOCK RT[8][BPI];
    BLOCK States[BPI];
    BLOCK S, T, t;

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
        States[0] = data[index  ];
        States[1] = data[index+1];
        States[2] = data[index+2];
        States[3] = data[index+3];
        DEOXYS( States, key, RT )   
        
        *Y = gf_2_128_double_four(*Y, States);
        accumulate_four_stateful(*X, States);

        index += BPI;
        --iters;
    }
    
    remaining = len % npbytes;
    while (remaining >= 16) {
        ctr = ADD_ONE(ctr); T = ctr; S = data[index];
        TAES(S, key, T, t);
        *X = XOR(*X, S);
        *Y = Double(*Y); *Y = XOR(*Y, S);   
        index += 1;
        remaining -= 16;
    }
    if (remaining > 0) { //If the last block is not full
        ctr = ADD_ONE(ctr); T = ctr; //DomainSep
        S = ZERO();
        memcpy(&S, data+index, remaining); //With 0* padding
        TAES(S, key, T, t);
        *X = XOR(*X, S);
        *Y = Double(*Y); *Y = XOR(*Y, S);   
    } 
    return ctr;
}


int prp_encrypt(prp_ctx     * restrict ctx,
               const void *pt,
               uint64_t    pt_len,
               const void *tk,
               uint64_t    tk_len,
               void       *ct,
               int        encrypt)
{
    BLOCK       * ctp = (BLOCK *)ct;
    const BLOCK * restrict ptp = (BLOCK *)pt;
    const BLOCK * restrict tkp = (BLOCK *)tk;
    BLOCK X, Y, Z, W, ctr;
    BLOCK T, t, S;

    uint64_t len1 = pt_len*8;
    uint64_t len2 = tk_len*8;
    BLOCK LEN = _mm_set_epi64x(len1, len2);

        
/* --------------- The Upper Hash using PMAC2x -------------------------*/
    X = ZERO();
    Y = ZERO();
    ctr = ZERO();
/* ---------------------------- Process Tweaks -------------------------*/
    ctr = phash(tkp, ctx->round_keys_h, tk_len, ctr, &X, &Y);
    BLOCK HT0 = X;
    BLOCK HT1 = Y;
    BLOCK CTR_SAVE = ctr;
/*-----------------------Process Plaintexts----------------------------*/
    ctr = phash(ptp+2, ctx->round_keys_h, (pt_len - 2*16), ctr, &X, &Y);
    ctr = ADD_ONE(ctr);
    T = XOR(ctr, ONE);
/*--------------------------------------------------------------------*/    
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
    
/*-----------Process The First Two Blocks----------------*/
    Z = XOR(ptp[0], Z);
    W = XOR(ptp[1], W);
    if(encrypt) {
#ifdef PRINT
    printf("U1: "); printreg(&Z, 16);
    printf("U2: "); printreg(&W, 16);
#endif
    TAES(Z, ctx->round_keys_1, W, t);
    T = W;
    TAES(T, ctx->round_keys_2, Z, t);
    W = XOR(T, W);
#ifdef PRINT
    printf("Z : "); printreg(&Z, 16);
    printf("W : "); printreg(&W, 16);
#endif
    S = Z;
    TAES(S, ctx->round_keys_3, T, t);
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
    TAESD(Z, ctx->round_keys_d_3, W, t);
    T = W;
    TAESD(T, ctx->round_keys_d_2, Z, t);
    W = XOR(T, W);
#ifdef PRINT
    printf("Z : "); printreg(&Z, 16);
    printf("W : "); printreg(&W, 16);
#endif
    S = Z;
    TAESD(S, ctx->round_keys_d_1, T, t);
#ifdef PRINT
    printf("U1: "); printreg(&S, 16);
    printf("U2: "); printreg(&T, 16);
#endif
    }
    ctp[0] = S; ctp[1] = T;
/*------------------------------- The CTR Part -------------------------*/
    ctr_mode(ptp + 2, ctx->round_keys_c, (pt_len - 2*16), W, Z, ctp+2);

/* ---------------- The Lower Hash using -------------------------*/
/*----------------------- Process Plaintexts ----------------------------*/
    ctr = CTR_SAVE; //Use Saved values from previous tweak process
    X = HT0;
    Y = HT1;
    ctr = phash(ctp+2, ctx->round_keys_h, (pt_len - 2*16), ctr, &X, &Y);
    
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

