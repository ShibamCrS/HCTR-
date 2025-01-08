#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

/* #define PRINT */
#include "../include/setup.h"
#include "../include/deoxysbc.h"
#include "../include/init.h"
#include "../include/utility.h"


void zhash(const BLOCK * data, const BLOCK key[DEOXYS_BC_128_256_NUM_ROUND_KEYS], uint64_t len, BLOCK *U, BLOCK *V, BLOCK *Ll, BLOCK *Lr) {
    uint64_t i, index;
    BLOCK SR[8][8];
    BLOCK SL[8];
    BLOCK S, T, t;
    while (len >= 256) {
        SL[0] = XOR(*Ll, data[index     ]); *Ll = Double(*Ll); 
        SL[1] = XOR(*Ll, data[index +  2]); *Ll = Double(*Ll); 
        SL[2] = XOR(*Ll, data[index +  4]); *Ll = Double(*Ll); 
        SL[3] = XOR(*Ll, data[index +  6]); *Ll = Double(*Ll); 
        SL[4] = XOR(*Ll, data[index +  8]); *Ll = Double(*Ll); 
        SL[5] = XOR(*Ll, data[index + 10]); *Ll = Double(*Ll); 
        SL[6] = XOR(*Ll, data[index + 12]); *Ll = Double(*Ll); 
        SL[7] = XOR(*Ll, data[index + 14]); *Ll = Double(*Ll); 

        SR[0][0]  = XOR(*Lr, data[index +  1]); *Lr = Double(*Lr); 
        SR[0][1]  = XOR(*Lr, data[index +  3]); *Lr = Double(*Lr); 
        SR[0][2]  = XOR(*Lr, data[index +  5]); *Lr = Double(*Lr); 
        SR[0][3]  = XOR(*Lr, data[index +  7]); *Lr = Double(*Lr); 
        SR[0][4]  = XOR(*Lr, data[index +  9]); *Lr = Double(*Lr); 
        SR[0][5]  = XOR(*Lr, data[index + 11]); *Lr = Double(*Lr); 
        SR[0][6]  = XOR(*Lr, data[index + 13]); *Lr = Double(*Lr); 
        SR[0][7]  = XOR(*Lr, data[index + 15]); *Lr = Double(*Lr); 
        
        SR[0][0] = TRUNC(SR[0][0],ONE);\
        SR[0][1] = TRUNC(SR[0][1],ONE);\
        SR[0][2] = TRUNC(SR[0][2],ONE);\
        SR[0][3] = TRUNC(SR[0][3],ONE);
        SR[0][4] = TRUNC(SR[0][4],ONE);
        SR[0][5] = TRUNC(SR[0][5],ONE);
        SR[0][6] = TRUNC(SR[0][6],ONE);
        SR[0][7] = TRUNC(SR[0][7],ONE);

        for(i=1; i<8; i++){ //UPDATE_TWEAK 
            SR[i][0] = PERMUTE(SR[i-1][0]);
            SR[i][1] = PERMUTE(SR[i-1][1]);
            SR[i][2] = PERMUTE(SR[i-1][2]);
            SR[i][3] = PERMUTE(SR[i-1][3]);
            SR[i][4] = PERMUTE(SR[i-1][4]);
            SR[i][5] = PERMUTE(SR[i-1][5]);
            SR[i][6] = PERMUTE(SR[i-1][6]);
            SR[i][7] = PERMUTE(SR[i-1][7]);
        }
        DEOXYS8( SL, key, SR )  //CL == SL

        *U = gf_2_128_double_eight(*U, SL);
        
        SL[0] = XOR(SL[0], data[index + 1]); \
        SL[1] = XOR(SL[1], data[index + 3]); \
        SL[2] = XOR(SL[2], data[index + 5]); \
        SL[3] = XOR(SL[3], data[index + 7]);
        SL[4] = XOR(SL[4], data[index + 9]);
        SL[5] = XOR(SL[5], data[index +11]);
        SL[6] = XOR(SL[6], data[index +13]);
        SL[7] = XOR(SL[7], data[index +15]);
        
        accumulate_eight_stateful(*V, SL);
                
        index += 16;
        len -= 256;
    }
    while (len >= 128) {
        SL[0] = XOR(*Ll, data[index     ]); *Ll = Double(*Ll); 
        SL[1] = XOR(*Ll, data[index +  2]); *Ll = Double(*Ll); 
        SL[2] = XOR(*Ll, data[index +  4]); *Ll = Double(*Ll); 
        SL[3] = XOR(*Ll, data[index +  6]); *Ll = Double(*Ll); 

        SR[0][0]  = XOR(*Lr, data[index +  1]); *Lr = Double(*Lr); 
        SR[0][1]  = XOR(*Lr, data[index +  3]); *Lr = Double(*Lr); 
        SR[0][2]  = XOR(*Lr, data[index +  5]); *Lr = Double(*Lr); 
        SR[0][3]  = XOR(*Lr, data[index +  7]); *Lr = Double(*Lr); 
        
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
        DEOXYS4( SL, key, SR )  //CL == SL

        *U = gf_2_128_double_four(*U, SL);
        
        SL[0] = XOR(SL[0], data[index + 1]); \
        SL[1] = XOR(SL[1], data[index + 3]); \
        SL[2] = XOR(SL[2], data[index + 5]); \
        SL[3] = XOR(SL[3], data[index + 7]);
        
        accumulate_four_stateful(*V, SL);
                
        index += 8;
        len  -= 128;
    }

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
    BLOCK U, V, Z, W, ctr;
    BLOCK T, t, S;
    
    uint64_t len1 = pt_len*8; 
    uint64_t len2 = tk_len*8; 
    BLOCK LEN = _mm_set_epi64x(len1, len2);

    
/* --------------- The Upper Hash using ZMAC2x -------------------------*/
    //Setup Ll and Lr
    BLOCK Ll = ZERO(); BLOCK Lr = ZERO();
    T = ZERO();
    TAES(Ll, ctx->round_keys_h, T, t);
    T = ADD_ONE(T);
    TAES(Lr, ctx->round_keys_h, T, t);

    U = ZERO();
    V = ZERO();

/* ---------------------------- Process Tweaks -------------------------*/
    zhash(tkp, ctx->round_keys_h, tk_len, &U, &V, &Ll, &Lr);
    
    BLOCK HT0 = U;
    BLOCK HT1 = V;
    BLOCK Ll_SAVE = Ll;
    BLOCK Lr_SAVE = Lr;
    
/*-----------------------Process Plaintexts----------------------------*/
    zhash(ptp+2, ctx->round_keys_h, (pt_len - 2*16), &U, &V, &Ll, &Lr);
    
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
/*-------------------------------- The CTR Part -------------------------*/
    ctr_mode(ptp + 2, ctx->round_keys_c, (pt_len - 2*16), W, Z, ctp + 2);
    
/* ---------------- The Lower Hash using PMAC2x -------------------------*/
/*----------------------- Process Plaintexts ----------------------------*/
    U = HT0;
    V = HT1;
    Ll = Ll_SAVE;
    Lr = Lr_SAVE;
    zhash(ctp + 2, ctx->round_keys_h, (pt_len - 2*16), &U, &V, &Ll, &Lr);
     
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
