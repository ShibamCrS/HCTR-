#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "../include/setup512.h"
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
void ctr_mode(const BLOCK *ptp, const BLOCK4 key4[DEOXYS_BC_128_256_NUM_ROUND_KEYS],  const BLOCK key[DEOXYS_BC_128_256_NUM_ROUND_KEYS], uint64_t len, BLOCK W, BLOCK Z, BLOCK *ctp) {
    const BLOCK4 * restrict ptp4 = (BLOCK4 *)ptp;
          BLOCK4 *          ctp4 = (BLOCK4 *)ctp;

    uint64_t index, index4;
    index  = 0;
    index4 = 0;
    BLOCK S, T, t, ctr;

    BLOCK4 state[2], RT[8], tmp, ctr4;
    ctr4 = CTR3210;

    BLOCK4 WW = zbroadcast(W);
    BLOCK4 ZZ = zbroadcast(Z);
    WW = XOR4(WW, key4[0]);

    while (len >= 128) {
        RT[0] = XOR4(ctr4, ZZ);
        UPDATE_TWEAK_ROUNDS_512_2(RT); // RT[1] .. RT[7] = permuted ctr XOR Z
        DEOXYS_FIXED_INPUT_512_2(state, key4, RT, WW, tmp);

        ctp4[index4    ] = XOR4(state[0], ptp4[index4    ]);
        ctp4[index4 + 1] = XOR4(state[1], ptp4[index4 + 1]);

        ctr4 = ADD4(ctr4, eight_512);

        index4 += 2;
        index  += 8;
        len -= 128;
    }

    while (len >= 64) {
        RT[0] = XOR4(ctr4, ZZ);
        UPDATE_TWEAK_ROUNDS_512(RT);
        DEOXYS_FIXED_INPUT_512(state, key4, RT, WW);

        ctp4[index4    ] = XOR4(state[0], ptp4[index4    ]);

        ctr4 = ADD4(ctr4, four_512);

        index4++;
        index  += 4;
        len -= 64;
    }
    /* ctr = _mm512_extracti64x2_epi64(ctr4, 0); */
    ctr = ((BLOCK *)(&ctr4))[0];
    while (len > 0) {
        T = XOR(ctr, Z);
        S = W;
        TAES(S, key, T, t);
        ctp[index] = XOR(S, ptp[index]);
        ctr = ADD_ONE(ctr); 
        index += 1;
        len -= 16;
    }
}
