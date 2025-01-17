#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "../include/setup.h"
#include "../include/deoxysbc.h"
#include "../include/init.h"
#include "../include/utility.h"

void printreg(const void *a, int nrof_byte){
    int i;
    unsigned char *f = (unsigned char *)a;
    for(i=0; i < nrof_byte; i++){
        printf("%02X",(unsigned char) f[nrof_byte - 1 -i]); //uint8_t c[4+8];
        if(nrof_byte > 16) if((i%16) == 15) printf(" ");
    }
    printf("\n");
}
//multiply by 2 over finite field
BLOCK Double(BLOCK b) {
    const BLOCK mask = _mm_set_epi32(135,1,1,1);
    BLOCK t = _mm_srai_epi32(b, 31);
    t = _mm_and_si128(t, mask);
    t = _mm_shuffle_epi32(t, _MM_SHUFFLE(2,1,0,3));
    b = _mm_slli_epi32(b, 1);
    return _mm_xor_si128(b,t);
}

/*
    computing:
    sum = 2(2(2(2Y + S[0]) + S[1]) + S[2]) + S[3]
        = 2^4Y + 2^3S[0] + 2^2S[1] + 2S[2] + S[3]
*/

BLOCK gf_2_128_double_four(BLOCK Y, BLOCK *S) {
    BLOCK tmp[4];
    tmp[0] = _mm_srli_epi64(Y   , 60);
    tmp[1] = _mm_srli_epi64(S[0], 61);
    tmp[2] = _mm_srli_epi64(S[1], 62);
    tmp[3] = _mm_srli_epi64(S[2], 63);

    BLOCK sum;
    accumulate_four(tmp, sum);

    BLOCK mod =  _mm_clmulepi64_si128(sum, REDUCTION_POLYNOMIAL, 0x01);

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
BLOCK gf_2_128_double_eight(BLOCK Y, BLOCK *S) {
    BLOCK tmp[8];
    tmp[0] = _mm_srli_epi64(Y   , 56);
    tmp[1] = _mm_srli_epi64(S[0], 57);
    tmp[2] = _mm_srli_epi64(S[1], 58);
    tmp[3] = _mm_srli_epi64(S[2], 59);
    tmp[4] = _mm_srli_epi64(S[3], 60);
    tmp[5] = _mm_srli_epi64(S[4], 61);
    tmp[6] = _mm_srli_epi64(S[5], 62);
    tmp[7] = _mm_srli_epi64(S[6], 63);

    BLOCK sum;
    accumulate_eight(tmp, sum);

    BLOCK mod =  _mm_clmulepi64_si128(sum, REDUCTION_POLYNOMIAL, 0x01);

    BLOCK sum_low = _mm_bslli_si128(sum, 8);

    tmp[0] = _mm_slli_epi64(Y,    8);
    tmp[1] = _mm_slli_epi64(S[0], 7);
    tmp[2] = _mm_slli_epi64(S[1], 6);
    tmp[3] = _mm_slli_epi64(S[2], 5);
    tmp[4] = _mm_slli_epi64(S[3], 4);
    tmp[5] = _mm_slli_epi64(S[4], 3);
    tmp[6] = _mm_slli_epi64(S[5], 2);
    tmp[7] = _mm_slli_epi64(S[6], 1);

    accumulate_eight(tmp, sum);
    sum = XOR(sum, sum_low);
    sum = XOR(sum, mod);
    sum = XOR(sum, S[7]);
    return sum;
}
void ctr_mode(const BLOCK *ptp, const BLOCK key[DEOXYS_BC_128_256_NUM_ROUND_KEYS], uint64_t len, BLOCK W, BLOCK Z, BLOCK *ctp) {
    const BLOCK4 * restrict ptp4 = (BLOCK4 *)ptp;
          BLOCK4 *          ctp4 = (BLOCK4 *)ctp;

    uint64_t index, index4, i;
    index  = 0;
    index4 = 0;
    BLOCK S, T, t, ctr;

    BLOCK4 state[2], RT[8][2], RTT[8], ctr4;

    BLOCK4 WW = _mm512_broadcast_i64x2(W);
    BLOCK4 ZZ = _mm512_broadcast_i64x2(Z);

    BLOCK4 key4[DEOXYS_BC_128_256_NUM_ROUND_KEYS];
    for(int i=0; i<=DEOXYS_BC_128_256_NUM_ROUNDS; i++){
        key4[i] = _mm512_broadcast_i64x2(key[i]);
    }

    ctr4 = CTR0123;
    /* while (len >= 256) { */
    /*     ctr4   =  ADD4444(ctr4); RT[0][0]  = XOR4(ZZ, ctr4); */
    /*     ctr4   =  ADD4444(ctr4); RT[0][1]  = XOR4(ZZ, ctr4); */
    /*     ctr4   =  ADD4444(ctr4); RT[0][2]  = XOR4(ZZ, ctr4); */
    /*     ctr4   =  ADD4444(ctr4); RT[0][3]  = XOR4(ZZ, ctr4); */

    /*     for(i=1; i<8; i++){ //UPDATE_TWEAK */ 
    /*         RT[i][0] = PERMUTE_512(RT[i-1][0]); */
    /*         RT[i][1] = PERMUTE_512(RT[i-1][1]); */
    /*         RT[i][2] = PERMUTE_512(RT[i-1][2]); */
    /*         RT[i][3] = PERMUTE_512(RT[i-1][3]); */
    /*     } */

    /*     state[0] = state[1] = state[2] = state[3] = WW; */
    /*     DEOXYS4( state, key4, RT ) */
    /*     ctp4[index4]   = XOR4(state[0], ptp4[index4]); */
    /*     ctp4[index4+1] = XOR4(state[1], ptp4[index4+1]); */
    /*     ctp4[index4+2] = XOR4(state[2], ptp4[index4+2]); */
    /*     ctp4[index4+3] = XOR4(state[3], ptp4[index4+3]); */

    /*     index  += 16; */
    /*     index4 += 4; */
    /*     len -= 256; */
    /* } */
    while (len >= 128) {
        ctr4   =  ADD4444(ctr4); RT[0][0]  = XOR4(ZZ, ctr4);
        ctr4   =  ADD4444(ctr4); RT[0][1]  = XOR4(ZZ, ctr4);

        for(i=1; i<8; i++){ //UPDATE_TWEAK 
            RT[i][0] = PERMUTE_512(RT[i-1][0]);
            RT[i][1] = PERMUTE_512(RT[i-1][1]);
        }

        state[0] = state[1] = WW;
        DEOXYS2( state, key4, RT )
        ctp4[index4] = XOR4(state[0], ptp4[index4]);
        ctp4[index4+1] = XOR4(state[1], ptp4[index4+1]);

        index  += 8;
        index4 += 2;
        len -= 128;
    }

    while (len >= 64) {
        ctr4   =  ADD4444(ctr4);
        RTT[0]  = XOR4(ZZ, ctr4);

        for(i=1; i<8; i++){ //UPDATE_TWEAK 
            RTT[i] = PERMUTE_512(RTT[i-1]);
        }

        state[0] = WW;
        DEOXYS( state[0], key4, RTT )
        ctp4[index4] = XOR4(state[0], ptp4[index4]);

        index  += 4;
        index4 += 1;
        len -= 64;
    }
    ctr = _mm512_extracti64x2_epi64(ctr4, 3);
    while (len > 0) {
        ctr = ADD_ONE(ctr); T = XOR(ctr, Z);
        S = W;
        TAES(S, key, T, t);
        ctp[index] = XOR(S, ptp[index]);
        index += 1;
        len -= 16;
    }
}
