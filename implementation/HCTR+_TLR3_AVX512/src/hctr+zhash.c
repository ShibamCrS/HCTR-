#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

/* #define PRINT */
#include "../include/setup512.h"
#include "../include/deoxysbc.h"
#include "../include/init.h"
#include "../include/utility.h"

//multiply by 4 over finite field
static inline BLOCK4 mult4by4(BLOCK4 b) {
    const BLOCK4 poly = _mm512_set_epi64(0,135, 0,135, 0,135, 0,135);
    const BLOCK4 mask = _mm512_set_epi64(15,0, 15,0, 15,0, 15,0);
    BLOCK4 t = _mm512_srli_epi64(b, 60);

    t = _mm512_shuffle_epi32(t, _MM_SHUFFLE(1,0,3,2));
    __m512i mod =  _mm512_clmulepi64_epi128(t, poly, 0x00);
    t = _mm512_and_si512(t, mask);

    b = _mm512_slli_epi64(b, 4);
    b = _mm512_xor_si512(b,t);
    b = _mm512_xor_si512(b,mod);
    return b;
}

void zhash(const BLOCK * data, const BLOCK4 key4[DEOXYS_BC_128_256_NUM_ROUND_KEYS], const BLOCK key[DEOXYS_BC_128_256_NUM_ROUND_KEYS], uint64_t len, BLOCK *U, BLOCK *V, BLOCK *Ll, BLOCK *Lr) {

    const BLOCK4 * restrict data4 = (BLOCK4 *)data;
    uint64_t index4, index, i;
    index = 0; index4 = 0;

    BLOCK T, t, S, Lll[4], Lrr[4];
    BLOCK4 *Ll4, *Lr4, A[4], B[4];

    BLOCK4 SL4[4], SR4[8][4], SRR4[8];

    Lll[0] = *Ll;             
    Lll[1] = Double(Lll[0]);             
    Lll[2] = Double(Lll[1]);             
    Lll[3] = Double(Lll[2]);             
    Ll4 = (BLOCK4 *)Lll;
    
    Lrr[0] = *Lr;             
    Lrr[1] = Double(Lrr[0]);             
    Lrr[2] = Double(Lrr[1]);             
    Lrr[3] = Double(Lrr[2]);             
    Lr4 = (BLOCK4 *)Lrr;
    
    BLOCK *SL = (BLOCK *)(&SL4);
    
    while (len >= 512) {
        A[0] = _mm512_shuffle_i64x2(data4[index4], data4[index4 + 1], _MM_SHUFFLE(2, 0, 2, 0));
        B[0] = _mm512_shuffle_i64x2(data4[index4], data4[index4 + 1], _MM_SHUFFLE(3, 1, 3, 1));
        A[1] = _mm512_shuffle_i64x2(data4[index4+2], data4[index4+3], _MM_SHUFFLE(2, 0, 2, 0));
        B[1] = _mm512_shuffle_i64x2(data4[index4+2], data4[index4+3], _MM_SHUFFLE(3, 1, 3, 1));
        A[2] = _mm512_shuffle_i64x2(data4[index4+4], data4[index4 + 5], _MM_SHUFFLE(2, 0, 2, 0));
        B[2] = _mm512_shuffle_i64x2(data4[index4+4], data4[index4 + 5], _MM_SHUFFLE(3, 1, 3, 1));
        A[3] = _mm512_shuffle_i64x2(data4[index4+6], data4[index4+7], _MM_SHUFFLE(2, 0, 2, 0));
        B[3] = _mm512_shuffle_i64x2(data4[index4+6], data4[index4+7], _MM_SHUFFLE(3, 1, 3, 1));

        SL4[0]    = XOR4(Ll4[0], A[0]);
        SR4[0][0] = XOR4(Lr4[0], B[0]);
        Ll4[0] = mult4by4(Ll4[0]);
        Lr4[0] = mult4by4(Lr4[0]);
       
        SL4[1]    = XOR4(Ll4[0], A[1]);
        SR4[0][1] = XOR4(Lr4[0], B[1]);
        Ll4[0] = mult4by4(Ll4[0]);
        Lr4[0] = mult4by4(Lr4[0]);

        SL4[2]    = XOR4(Ll4[0], A[2]);
        SR4[0][2] = XOR4(Lr4[0], B[2]);
        Ll4[0] = mult4by4(Ll4[0]);
        Lr4[0] = mult4by4(Lr4[0]);
        
        SL4[3]    = XOR4(Ll4[0], A[3]);
        SR4[0][3] = XOR4(Lr4[0], B[3]);
        Ll4[0] = mult4by4(Ll4[0]);
        Lr4[0] = mult4by4(Lr4[0]);

        SR4[0][0] = TRUNC4(SR4[0][0], ONE4);
        SR4[0][1] = TRUNC4(SR4[0][1], ONE4);
        SR4[0][2] = TRUNC4(SR4[0][2], ONE4);
        SR4[0][3] = TRUNC4(SR4[0][3], ONE4);
        for(i=1; i<8; i++){ //UPDATE_TWEAK
            SR4[i][0] = PERMUTE_512(SR4[i-1][0]);
            SR4[i][1] = PERMUTE_512(SR4[i-1][1]);
            SR4[i][2] = PERMUTE_512(SR4[i-1][2]);
            SR4[i][3] = PERMUTE_512(SR4[i-1][3]);
        }
        DEOXYS4( SL4, key4, SR4 )
        *U = gf_2_128_double_16(*U, SL);
        XOR4_four(SL4, B);

        accumulate_16_stateful(*V, SL);

        index  += 32;
        index4 += 8;
        len -= 512;
    }
    while (len >= 256) {
        A[0] = _mm512_shuffle_i64x2(data4[index4], data4[index4 + 1], _MM_SHUFFLE(2, 0, 2, 0));
        B[0] = _mm512_shuffle_i64x2(data4[index4], data4[index4 + 1], _MM_SHUFFLE(3, 1, 3, 1));
        A[1] = _mm512_shuffle_i64x2(data4[index4+2], data4[index4+3], _MM_SHUFFLE(2, 0, 2, 0));
        B[1] = _mm512_shuffle_i64x2(data4[index4+2], data4[index4+3], _MM_SHUFFLE(3, 1, 3, 1));
        
        SL4[0]    = XOR4(Ll4[0], A[0]);
        SR4[0][0] = XOR4(Lr4[0], B[0]);
        Ll4[0] = mult4by4(Ll4[0]);
        Lr4[0] = mult4by4(Lr4[0]);
       
        SL4[1]    = XOR4(Ll4[0], A[1]);
        SR4[0][1] = XOR4(Lr4[0], B[1]);
        Ll4[0] = mult4by4(Ll4[0]);
        Lr4[0] = mult4by4(Lr4[0]);

        /* TRUNC4_two(SR4[0][0], SR4[1][0]); */
        SR4[0][0] = TRUNC4(SR4[0][0], ONE4);
        SR4[0][1] = TRUNC4(SR4[0][1], ONE4);
        for(i=1; i<8; i++){ //UPDATE_TWEAK
            SR4[i][0] = PERMUTE_512(SR4[i-1][0]);
            SR4[i][1] = PERMUTE_512(SR4[i-1][1]);
        }
        DEOXYS2( SL4, key4, SR4 )
        *U = gf_2_128_double_eight(*U, SL);
        XOR4_two(SL4, B);
        accumulate_eight_stateful(*V, SL);

        index  += 16;
        index4 += 4;
        len -= 256;
    }

    while (len >= 128) {
        A[0] = _mm512_shuffle_i64x2(data4[index4], data4[index4 + 1], _MM_SHUFFLE(2, 0, 2, 0));
        B[0] = _mm512_shuffle_i64x2(data4[index4], data4[index4 + 1], _MM_SHUFFLE(3, 1, 3, 1));
        SL4[0]  = XOR4(Ll4[0], A[0]);
        SRR4[0] = XOR4(Lr4[0], B[0]);
        Ll4[0]  = mult4by4(Ll4[0]);
        Lr4[0]  = mult4by4(Lr4[0]);

        SRR4[0] = TRUNC4(SRR4[0], ONE4);
        for(i=1; i<8; i++){ //UPDATE_TWEAK
            SRR4[i] = PERMUTE_512(SRR4[i-1]);
        }
        DEOXYS( SL4[0], key4, SRR4 )
        *U = gf_2_128_double_four(*U, SL);
        SL4[0] = XOR4(SL4[0], B[0]);
        accumulate_four_stateful(*V, SL);

        index  += 8;
        index4 += 2;
        len -= 128;
    }

    *Ll = ((BLOCK *)Ll4)[0]; 
    *Lr = ((BLOCK *)Lr4)[0]; 

    while (len >= 32) {
        S = XOR(*Ll, data[index]);      *Ll = Double(*Ll);
        T = XOR(*Lr, data[index +  1]); *Lr = Double(*Lr);
        TAES(S, key, T, t);
        *U = Double(*U); *U = XOR(*U, S);
        *V = XOR(*V, XOR(S, data[index+1]));
        index += 2;
        len -= 32;
    }
    if (len > 0) {
        SL[0] = ZERO();
        SL[1] = ZERO();
        memcpy(SL, data+index, len);
        S = XOR(*Ll, SL[0]); *Ll = Double(*Ll);
        T = XOR(*Lr, SL[1]); *Lr = Double(*Lr);
        TAES(S, key, T, t);

        *U = Double(*U); *U = XOR(*U, S);
        *V = XOR(*V,  XOR(S, SL[1]));
    }
}

void prp_encrypt(prp_ctx     * restrict ctx,
               const void *pt,
               uint64_t    pt_len,
               const void *tk,
               uint64_t    tk_len,
               void       *ct,
               int        encrypt)
{
    BLOCK       *          ctp = (BLOCK *)ct;
    const BLOCK * restrict ptp = (BLOCK *)pt;
    const BLOCK * restrict tkp = (BLOCK *)tk;

    BLOCK U, V, Z, W;
    BLOCK T, t, S;

    uint64_t len1 = pt_len*8;
    uint64_t len2 = tk_len*8;
    BLOCK LEN = _mm_set_epi64x(len1, len2);
    
    //Setup Ll and Lr
    BLOCK Ll, Lr;
    Ll = ZERO(); Lr = ZERO();
    T = ZERO();
    TAES(Ll, ctx->round_keys_h, T, t);
    T = ADD_ONE(T);
    TAES(Lr, ctx->round_keys_h, T, t);
    
    U = ZERO();
    V = ZERO();

/* ---------------------------- Process Tweaks -------------------------*/
    zhash(tkp, ctx->round_keys_h_512, ctx->round_keys_h, tk_len, &U, &V, &Ll, &Lr);

    BLOCK HT0 = U;
    BLOCK HT1 = V;
    BLOCK Ll_SAVE = Ll;
    BLOCK Lr_SAVE = Lr;

/*-----------------------Process Plaintexts----------------------------*/
    zhash(ptp+2, ctx->round_keys_h_512, ctx->round_keys_h, (pt_len - 2*16), &U, &V, &Ll, &Lr);

    //Handel Length
    S = XOR(Ll, LEN);
    TAES(S, ctx->round_keys_h, Lr, t);
    U = Double(U); U = XOR(U, S);
    V = XOR(V, S);
    //FInalization Of Hash
    Z = U; W = V;
    V = TRUNC(V, TWO); U = TRUNC(U, THREE);
    TAES(Z, ctx->round_keys_h, V, t);
    TAES(W, ctx->round_keys_h, U, t);

/*-----------------Process Upper Part Of The First Two Blocks----------------*/
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
/*-------------------------------- The CTR Part -------------------------*/
    /* ctr_mode(ptp + 2, ctx->round_keys_c_512, ctx->round_keys_c, (pt_len - 2*16), W, Z, ctp + 2); */
    xorpp_mode(ptp + 2, ctx->round_keys_c_512, ctx->round_keys_c, (pt_len - 2*16), W, Z, ctp + 2);

/* ---------------- The Lower Hash using PMAC2x -------------------------*/
/*----------------------- Process Plaintexts ----------------------------*/
    U = HT0;
    V = HT1;
    Ll = Ll_SAVE;
    Lr = Lr_SAVE;
    zhash(ctp + 2, ctx->round_keys_h_512, ctx->round_keys_h, (pt_len - 2*16), &U, &V, &Ll, &Lr);
    
    //Handel Length 
    S = XOR(Ll, LEN);
    TAES(S, ctx->round_keys_h, Lr, t);
    U = Double(U); U = XOR(U, S);
    V = XOR(V, S);
/*----------------- Process First Two Blocks ----------------------*/
    //FInalization Of Hash
    Z = U; W = V;
    V = TRUNC(V, TWO); U = TRUNC(U, THREE);
    TAES(Z, ctx->round_keys_h, V, t);
    TAES(W, ctx->round_keys_h, U, t);

    ctp[0] = XOR(ctp[0], Z);
    ctp[1] = XOR(ctp[1], W);
}
