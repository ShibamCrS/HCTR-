#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

#include "../include/setup512.h"
#include "../include/deoxysbc.h"
#include "../include/init.h"
#include "../include/utility.h"
/* #define PRINT */

BLOCK phash(const BLOCK * restrict data, const BLOCK4 key4[DEOXYS_BC_128_256_NUM_ROUND_KEYS], const BLOCK key[DEOXYS_BC_128_256_NUM_ROUND_KEYS], uint64_t len, BLOCK ctr, BLOCK *X, BLOCK *Y) {
    const BLOCK4 * restrict data4 = (BLOCK4 *)data;
    uint64_t index4, index;
    index = 0; index4 = 0;
    BLOCK S, T, t;

    BLOCK4 RT[8], state[2], ctr4, tmp;

    ctr4 = _mm512_broadcast_i64x2(ctr);
    while (len >= 128) {
        RT[0] = ctr4;
        UPDATE_TWEAK_ROUNDS_512_2(RT);
        ctr4 = ADD4(ctr4, eight_512);
        state[0] = data4[index4];
        state[1] = data4[index4+1];
        DEOXYS_HASH_INPUT_512_2(state, key4, RT, tmp);
        BLOCK *states = (BLOCK *)(&state);

        *Y = gf_2_128_double_eight(*Y, states);
        accumulate_eight_stateful(*X, states);

        len -= 128;
        index  += 8;
        index4 += 2;
    }
    while (len >= 64) {
        RT[0] = ctr4;
        UPDATE_TWEAK_ROUNDS_512(RT);
        ctr4 = ADD4(ctr4, four_512);
        state[0] = data4[index4];
        DEOXYS_HASH_INPUT_512(state, key4, RT);
        
        BLOCK *states = (BLOCK *)(&state);

        *Y = gf_2_128_double_four(*Y, states);
        accumulate_four_stateful(*X, states);

        len -= 64;
        index  += 4;
        index4 += 1;
    }
    ctr = ((BLOCK *)(&ctr4))[0];
    while (len >= 16) {
        T = ctr; S = data[index];
        ctr = ADD_ONE(ctr); 
        TAES(S, key, T, t);
        *X = XOR(*X, S);
        *Y = Double(*Y); *Y = XOR(*Y, S);
        len -= 16;
        index += 1;
    }
    if (len > 0) { //If the last block is not full
        T = ctr;
        ctr = ADD_ONE(ctr); 
        S = ZERO();
        memcpy(&S, data+index, len);
        TAES(S, key, T, t);
        *X = XOR(*X, S);
        *Y = Double(*Y); *Y = XOR(*Y, S);
    }
    return ctr;
}
void prp_encrypt(prp_ctx     * restrict ctx,
               const void *pt,
               uint64_t    pt_len,
               const void *tk,
               uint64_t    tk_len,
               void       *ct,
               int        encrypt)
{
    BLOCK       * ctp = (BLOCK *)ct;
    const BLOCK * ptp = (BLOCK *)pt;
    const BLOCK * tkp = (BLOCK *)tk;

    uint64_t len1 = pt_len*8;
    uint64_t len2 = tk_len*8;
    BLOCK LEN = _mm_set_epi64x(len1, len2);

    BLOCK X, Y, Z, W, ctr;
    BLOCK T, t, S;
        
/* --------------- The Upper Hash using PMAC2x -------------------------*/
    X = ZERO();
    Y = ZERO();
    ctr = ZERO();
/* ---------------------------- Process Tweaks -------------------------*/
    ctr = phash(tkp, ctx->round_keys_h_512, ctx->round_keys_h, tk_len, ctr, &X, &Y);
    BLOCK HT0 = X;
    BLOCK HT1 = Y;
    BLOCK CTR_SAVE = ctr;
/*-----------------------Process Plaintexts----------------------------*/
    ctr = phash(ptp+2, ctx->round_keys_h_512, ctx->round_keys_h, (pt_len - 2*16), ctr, &X, &Y);
/*--------------------------------------------------------------------*/
    //Handel Length 
    S = LEN;
    TAES(S, ctx->round_keys_h, ctr, t);
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
    TAES(Z, ctx->round_keys_1, W, t);
    T = W;
    TAES(T, ctx->round_keys_2, Z, t);
    W = XOR(T, W);
    S = Z;
    TAES(S, ctx->round_keys_3, T, t);
    }
    else {
    TAESD(Z, ctx->round_keys_d_3, W, t);
    T = W;
    TAESD(T, ctx->round_keys_d_2, Z, t);
    W = XOR(T, W);
    S = Z;
    TAESD(S, ctx->round_keys_d_1, T, t);
    }
    ctp[0] = S; ctp[1] = T;

/*------------------------------- The CTR Part -------------------------*/
    ctr_mode(ptp + 2, ctx->round_keys_c_512, ctx->round_keys_c, (pt_len - 2*16), W, Z, ctp+2);
/* ---------------- The Lower Hash using -------------------------*/
/*----------------------- Process Plaintexts ----------------------------*/
    ctr = CTR_SAVE; //Use Saved values from previous tweak process
    X = HT0;
    Y = HT1;
    ctr = phash(ctp+2, ctx->round_keys_h_512, ctx->round_keys_h, (pt_len - 2*16), ctr, &X, &Y);

    //Handel Length 
    S = LEN;
    TAES(S, ctx->round_keys_h, ctr, t);
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
}

