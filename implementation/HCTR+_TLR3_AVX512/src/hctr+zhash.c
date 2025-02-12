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

/* void zhash(const BLOCK * data, BLOCK4 key4[DEOXYS_BC_128_256_NUM_ROUND_KEYS], const BLOCK key[DEOXYS_BC_128_256_NUM_ROUND_KEYS], uint64_t len, BLOCK *U, BLOCK *V, BLOCK *Ll, BLOCK *Lr) { */

/*     const BLOCK4 * restrict data4 = (BLOCK4 *)data; */
/*     uint64_t index4, index, i; */
/*     index = 0; index4 = 0; */

/*     BLOCK T, t, S; */ 
/*     BLOCK Llr[8]; */
/*     BLOCK4 *Llr4, A, B; */
    
/*     BLOCK SR[8], SL[8]; */
/*     BLOCK4 SL4[2], SR4_0[8], SR4_1[8], tmp; */
        
/*     Llr[0] = *Ll;             Llr[1] = *Lr; */ 
/*     Llr[2] = Double(Llr[0]);  Llr[3] = Double(Llr[1]); */
/*     Llr[4] = Double(Llr[2]);  Llr[5] = Double(Llr[3]); */
/*     Llr[6] = Double(Llr[4]);  Llr[7] = Double(Llr[5]); */
/*     Llr4 = (BLOCK4 *)Llr; */
/*     while (len >= 256) { */
/*         A = XOR4(Llr4[0], data4[index4]); B = XOR4(Llr4[1], data4[index4 + 1]); */
/*         Llr4[0] = mult4by4(Llr4[0]); Llr4[1] = mult4by4(Llr4[1]); */
/*         // Shuffle to get C = [B2, B0, A2, A0] */
/*         SL4[0]    = _mm512_shuffle_i64x2(A, B, _MM_SHUFFLE(2, 0, 2, 0)); */
/*         // Shuffle to get D = [B3, B1, A3, A1] */
/*         SR4_0[0] = _mm512_shuffle_i64x2(A, B, _MM_SHUFFLE(3, 1, 3, 1)); */

/*         A = XOR4(Llr4[0], data4[index4+2]); B = XOR4(Llr4[1], data4[index4+3]); */
/*         Llr4[0] = mult4by4(Llr4[0]); Llr4[1] = mult4by4(Llr4[1]); */
/*         // Shuffle to get C = [B2, B0, A2, A0] */
/*         SL4[1]    = _mm512_shuffle_i64x2(A, B, _MM_SHUFFLE(2, 0, 2, 0)); */
/*         // Shuffle to get D = [B3, B1, A3, A1] */
/*         SR4_1[0] = _mm512_shuffle_i64x2(A, B, _MM_SHUFFLE(3, 1, 3, 1)); */

/*         SR4_0[0] = TRUNC4(SR4_0[0], ONE4); */
/*         SR4_1[0] = TRUNC4(SR4_1[0], ONE4); */
/*         UPDATE_TWEAK_ROUNDS_512(SR4_0); */
/*         UPDATE_TWEAK_ROUNDS_512(SR4_1); */
/*         DEOXYS_HASH_INPUT_512_z(SL4[0], key4, SR4_0); */
/*         DEOXYS_HASH_INPUT_512_z(SL4[1], key4, SR4_1); */

/*         BLOCK *SL = (BLOCK *)(&SL4); */

/*         *U = gf_2_128_double_eight(*U, SL); */

/*         SL[0] = XOR(SL[0], data[index + 1]); \ */
/*         SL[1] = XOR(SL[1], data[index + 3]); \ */
/*         SL[2] = XOR(SL[2], data[index + 5]); \ */
/*         SL[3] = XOR(SL[3], data[index + 7]); */
/*         SL[4] = XOR(SL[4], data[index + 9]); */
/*         SL[5] = XOR(SL[5], data[index + 11]); */
/*         SL[6] = XOR(SL[6], data[index + 13]); */
/*         SL[7] = XOR(SL[7], data[index + 15]); */

/*         accumulate_eight_stateful(*V, SL); */

/*         index  += 16; */
/*         index4 += 4; */
/*         len -= 256; */
/*     } */

/*     while (len >= 128) { */
/*         A = XOR4(Llr4[0], data4[index4]); B = XOR4(Llr4[1], data4[index4 + 1]); */
/*         Llr4[0] = mult4by4(Llr4[0]); Llr4[1] = mult4by4(Llr4[1]); */
/*         // Shuffle to get C = [B2, B0, A2, A0] */
/*         SL4[0]    = _mm512_shuffle_i64x2(A, B, _MM_SHUFFLE(2, 0, 2, 0)); */
/*         // Shuffle to get D = [B3, B1, A3, A1] */
/*         SR4_0[0] = _mm512_shuffle_i64x2(A, B, _MM_SHUFFLE(3, 1, 3, 1)); */

/*         SR4_0[0] = TRUNC4(SR4_0[0], ONE4); */
/*         UPDATE_TWEAK_ROUNDS_512(SR4_0); */
/*         DEOXYS_HASH_INPUT_512_z(SL4[0], key4, SR4_0); */

/*         BLOCK *SL = (BLOCK *)(&SL4); */
/*         *U = gf_2_128_double_four(*U, SL); */
/*         SL[0] = XOR(SL[0], data[index + 1]); \ */
/*         SL[1] = XOR(SL[1], data[index + 3]); \ */
/*         SL[2] = XOR(SL[2], data[index + 5]); \ */
/*         SL[3] = XOR(SL[3], data[index + 7]); */

/*         accumulate_four_stateful(*V, SL); */

/*         index  += 8; */
/*         index4 += 2; */
/*         len -= 128; */
/*     } */
/*     *Ll = _mm512_extracti64x2_epi64(Llr4[0], 0); */
/*     *Lr = _mm512_extracti64x2_epi64(Llr4[0], 1); */
    
/*     while (len >= 32) { */
/*         S = XOR(*Ll, data[index]);      *Ll = Double(*Ll); */
/*         T = XOR(*Lr, data[index +  1]); *Lr = Double(*Lr); */
/*         TAES(S, key, T, t); */
/*         *U = Double(*U); *U = XOR(*U, S); */
/*         *V = XOR(*V, XOR(S, data[index+1])); */
/*         index += 2; */
/*         len -= 32; */
/*     } */
/*     /1* if (len > 0) { *1/ */
/*     /1*     SL[0] = ZERO(); *1/ */
/*     /1*     SL[1] = ZERO(); *1/ */
/*     /1*     memcpy(SL, data+index, len); *1/ */
/*     /1*     S = XOR(*Ll, SL[0]); *Ll = Double(*Ll); *1/ */
/*     /1*     T = XOR(*Lr, SL[1]); *Lr = Double(*Lr); *1/ */
/*     /1*     TAES(S, key, T, t); *1/ */

/*     /1*     *U = Double(*U); *U = XOR(*U, S); *1/ */
/*     /1*     *V = XOR(*V,  XOR(S, SL[1])); *1/ */
/*     /1* } *1/ */
/* } */

void zhash(const BLOCK * data, const BLOCK4 key4[DEOXYS_BC_128_256_NUM_ROUND_KEYS], const BLOCK key[DEOXYS_BC_128_256_NUM_ROUND_KEYS], uint64_t len, BLOCK *U, BLOCK *V, BLOCK *Ll, BLOCK *Lr) {

    const BLOCK4 * restrict data4 = (BLOCK4 *)data;
    uint64_t index4, index, i;
    index = 0; index4 = 0;

    BLOCK T, t, S;
    BLOCK Llr[8];
    BLOCK4 *Llr4, A, B;

    BLOCK SR[8], SL[8];
    BLOCK4 SL4, SR4[8], SLL4[2], SRR4[8][2];


    Llr[0] = *Ll;             Llr[1] = *Lr;
    Llr[2] = Double(Llr[0]);  Llr[3] = Double(Llr[1]);
    Llr[4] = Double(Llr[2]);  Llr[5] = Double(Llr[3]);
    Llr[6] = Double(Llr[4]);  Llr[7] = Double(Llr[5]);
    Llr4 = (BLOCK4 *)Llr;

    /* BLOCK4 key4[DEOXYS_BC_128_256_NUM_ROUND_KEYS]; */
    /* for(int i=0; i<=DEOXYS_BC_128_256_NUM_ROUNDS; i++){ */
    /*     key4[i] = _mm512_broadcast_i64x2(key[i]); */
    /* } */
    while (len >= 256) {
        A = XOR4(Llr4[0], data4[index4]); B = XOR4(Llr4[1], data4[index4 + 1]);
        Llr4[0] = mult4by4(Llr4[0]); Llr4[1] = mult4by4(Llr4[1]);
        // Shuffle to get C = [B2, B0, A2, A0]
        SLL4[0]    = _mm512_shuffle_i64x2(A, B, _MM_SHUFFLE(2, 0, 2, 0));
        // Shuffle to get D = [B3, B1, A3, A1]
        SRR4[0][0] = _mm512_shuffle_i64x2(A, B, _MM_SHUFFLE(3, 1, 3, 1));

        A = XOR4(Llr4[0], data4[index4+2]); B = XOR4(Llr4[1], data4[index4+3]);
        Llr4[0] = mult4by4(Llr4[0]); Llr4[1] = mult4by4(Llr4[1]);
        // Shuffle to get C = [B2, B0, A2, A0]
        SLL4[1]    = _mm512_shuffle_i64x2(A, B, _MM_SHUFFLE(2, 0, 2, 0));
        // Shuffle to get D = [B3, B1, A3, A1]
        SRR4[0][1] = _mm512_shuffle_i64x2(A, B, _MM_SHUFFLE(3, 1, 3, 1));

        SRR4[0][0] = TRUNC4(SRR4[0][0], ONE4);
        SRR4[0][1] = TRUNC4(SRR4[0][1], ONE4);
        for(i=1; i<8; i++){ //UPDATE_TWEAK
            SRR4[i][0] = PERMUTE_512(SRR4[i-1][0]);
            SRR4[i][1] = PERMUTE_512(SRR4[i-1][1]);
        }
        DEOXYS2( SLL4, key4, SRR4 )

        /* SL[0] = _mm512_extracti64x2_epi64(SL4, 0); */
        /* SL[1] = _mm512_extracti64x2_epi64(SL4, 1); */
        /* SL[2] = _mm512_extracti64x2_epi64(SL4, 2); */
        /* SL[3] = _mm512_extracti64x2_epi64(SL4, 3); */
        BLOCK *SL = (BLOCK *)(&SLL4);

        *U = gf_2_128_double_eight(*U, SL);

        SL[0] = XOR(SL[0], data[index + 1]); \
        SL[1] = XOR(SL[1], data[index + 3]); \
        SL[2] = XOR(SL[2], data[index + 5]); \
        SL[3] = XOR(SL[3], data[index + 7]);
        SL[4] = XOR(SL[4], data[index + 9]);
        SL[5] = XOR(SL[5], data[index + 11]);
        SL[6] = XOR(SL[6], data[index + 13]);
        SL[7] = XOR(SL[7], data[index + 15]);

        accumulate_eight_stateful(*V, SL);

        index  += 16;
        index4 += 4;
        len -= 256;
    }

    while (len >= 128) {
        A = XOR4(Llr4[0], data4[index4]); B = XOR4(Llr4[1], data4[index4 + 1]);
        Llr4[0] = mult4by4(Llr4[0]); Llr4[1] = mult4by4(Llr4[1]);
        // Shuffle to get C = [B2, B0, A2, A0]
        SL4    = _mm512_shuffle_i64x2(A, B, _MM_SHUFFLE(2, 0, 2, 0));
        // Shuffle to get D = [B3, B1, A3, A1]
        SR4[0] = _mm512_shuffle_i64x2(A, B, _MM_SHUFFLE(3, 1, 3, 1));

        SR4[0] = TRUNC4(SR4[0], ONE4);
        for(i=1; i<8; i++){ //UPDATE_TWEAK
            SR4[i] = PERMUTE_512(SR4[i-1]);
        }
        DEOXYS( SL4, key4, SR4 )

        /* SL[0] = _mm512_extracti64x2_epi64(SL4, 0); */
        /* SL[1] = _mm512_extracti64x2_epi64(SL4, 1); */
        /* SL[2] = _mm512_extracti64x2_epi64(SL4, 2); */
        /* SL[3] = _mm512_extracti64x2_epi64(SL4, 3); */
        BLOCK *SL = (BLOCK *)(&SL4);

        *U = gf_2_128_double_four(*U, SL);

        SL[0] = XOR(SL[0], data[index + 1]); \
        SL[1] = XOR(SL[1], data[index + 3]); \
        SL[2] = XOR(SL[2], data[index + 5]); \
        SL[3] = XOR(SL[3], data[index + 7]);

        accumulate_four_stateful(*V, SL);

        index  += 8;
        index4 += 2;
        len -= 128;
    }
    /* *Ll = _mm512_extracti64x2_epi64(Llr4[0], 0); */
    /* *Lr = _mm512_extracti64x2_epi64(Llr4[0], 1); */

    /* while (len >= 32) { */
    /*     S = XOR(*Ll, data[index]);      *Ll = Double(*Ll); */
    /*     T = XOR(*Lr, data[index +  1]); *Lr = Double(*Lr); */
    /*     TAES(S, key, T, t); */
    /*     *U = Double(*U); *U = XOR(*U, S); */
    /*     *V = XOR(*V, XOR(S, data[index+1])); */
    /*     index += 2; */
    /*     len -= 32; */
    /* } */
    /* if (len > 0) { */
    /*     SL[0] = ZERO(); */
    /*     SL[1] = ZERO(); */
    /*     memcpy(SL, data+index, len); */
    /*     S = XOR(*Ll, SL[0]); *Ll = Double(*Ll); */
    /*     T = XOR(*Lr, SL[1]); *Lr = Double(*Lr); */
    /*     TAES(S, key, T, t); */

    /*     *U = Double(*U); *U = XOR(*U, S); */
    /*     *V = XOR(*V,  XOR(S, SL[1])); */
    /* } */
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
    BLOCK       *          ctp = (BLOCK *)ct;
    const BLOCK * restrict ptp = (BLOCK *)pt;
    const BLOCK * restrict tkp = (BLOCK *)tk;

    BLOCK U, V, Z, W, ctr;
    BLOCK T, t, S;

    uint64_t len1 = pt_len*8;
    uint64_t len2 = tk_len*8;
    BLOCK LEN = _mm_set_epi64x(len1, len2);
    
    BLOCK4 round_keys_h[DEOXYS_BC_128_256_NUM_ROUND_KEYS];
    BLOCK4 round_keys_c[DEOXYS_BC_128_256_NUM_ROUND_KEYS];
    for(int i=0; i<=DEOXYS_BC_128_256_NUM_ROUNDS; i++){
        round_keys_h[i] = _mm512_broadcast_i64x2(ctx->round_keys_h[i]);
        round_keys_c[i] = _mm512_broadcast_i64x2(ctx->round_keys_c[i]);
    }
    
    //Setup Ll and Lr
    BLOCK Ll, Lr;
    Ll = ZERO(); Lr = ZERO();
    T = ZERO();
    TAES(Ll, ctx->round_keys_h, T, t);
    T = ADD_ONE(T);
    TAES(Lr, ctx->round_keys_h, T, t);
    BLOCK XX[4];
    
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
#ifdef PRINT
    printf("U1: "); printregm(&Z, 16);
    printf("U2: "); printregm(&W, 16);
#endif
    TAES(Z, ctx->round_keys_1, W, t);
    T = W;
    TAES(T, ctx->round_keys_2, Z, t);
    W = XOR(T, W);
#ifdef PRINT
    printf("Z : "); printregm(&Z, 16);
    printf("W : "); printregm(&W, 16);
#endif
    S = Z;
    TAES(S, ctx->round_keys_3, T, t);
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
    TAESD(Z, ctx->round_keys_d_3, W, t);
    T = W;
    TAESD(T, ctx->round_keys_d_2, Z, t);
    W = XOR(T, W);
#ifdef PRINT
    printf("Z : "); printregm(&Z, 16);
    printf("W : "); printregm(&W, 16);
#endif
    S = Z;
    TAESD(S, ctx->round_keys_d_1, T, t);
#ifdef PRINT
    printf("U1: "); printregm(&S, 16);
    printf("U2: "); printregm(&T, 16);
#endif
    }
    ctp[0] = S; ctp[1] = T;
/*-------------------------------- The CTR Part -------------------------*/
    ctr_mode(ptp + 2, ctx->round_keys_c_512, ctx->round_keys_c, (pt_len - 2*16), W, Z, ctp + 2);

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

    return (int) pt_len;
}
