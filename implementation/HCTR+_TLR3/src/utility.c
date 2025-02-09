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
    /* __m128i a = vshiftl16(b, 1); // y = x << 1 */
    /* __m128i c = vshiftr16_sign(vshiftr_bytes(b, 14), 15); */
    /* __m128i msb = vand(set32(0, 0, 0, 0x87), c); */
    /* __m128i v = vshiftr16(vshiftl_bytes(b, 2), 15); */
    /* return vxor(vor(a, v), msb); */


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
    
    BLOCK RT[8];
    BLOCK States[8];
    BLOCK tmp;
    BLOCK ctr = one_be;

    uint64_t index = 0;

    W = XOR(W, key[0]);
    while (len >= 128) {
        RT[0] = XOR(ctr, Z);
        UPDATE_TWEAK_ROUNDS8(RT); // RT[1] .. RT[7] = permuted ctr XOR Z

        DEOXYS8_FIXED_INPUT(States, key, RT, W, tmp);

        ctp[index    ] = XOR(States[0], ptp[index    ]);
        ctp[index + 1] = XOR(States[1], ptp[index + 1]);
        ctp[index + 2] = XOR(States[2], ptp[index + 2]);
        ctp[index + 3] = XOR(States[3], ptp[index + 3]);
        ctp[index + 4] = XOR(States[4], ptp[index + 4]);
        ctp[index + 5] = XOR(States[5], ptp[index + 5]);
        ctp[index + 6] = XOR(States[6], ptp[index + 6]);
        ctp[index + 7] = XOR(States[7], ptp[index + 7]);

        ctr = vadd32(ctr, eight_be);

        index += 8;
        len -= 128;
    }

    while (len >= 64) {
        RT[0] = XOR(ctr, Z);
        UPDATE_TWEAK_ROUNDS4(RT);
        DEOXYS4_FIXED_INPUT(States, key, RT, W, tmp);

        ctp[index    ] = XOR(States[0], ptp[index    ]);
        ctp[index + 1] = XOR(States[1], ptp[index + 1]);
        ctp[index + 2] = XOR(States[2], ptp[index + 2]);
        ctp[index + 3] = XOR(States[3], ptp[index + 3]);

        ctr = vadd32(ctr, four_be);

        index += 4;
        len -= 64;
    }
    
    /* uint64_t index, i; */
    /* index = 0; */
    /* BLOCK RT[8][8]; */
    /* BLOCK States[8]; */

    /* BLOCK ctr = ZERO(); */

    /* while (len >= 128) { */
    /*     ctr =  ADD_ONE(ctr); RT[0][0] = XOR(ctr, Z); */
    /*     ctr =  ADD_ONE(ctr); RT[0][1] = XOR(ctr, Z); */
    /*     ctr =  ADD_ONE(ctr); RT[0][2] = XOR(ctr, Z); */
    /*     ctr =  ADD_ONE(ctr); RT[0][3] = XOR(ctr, Z); */
    /*     ctr =  ADD_ONE(ctr); RT[0][4] = XOR(ctr, Z); */
    /*     ctr =  ADD_ONE(ctr); RT[0][5] = XOR(ctr, Z); */
    /*     ctr =  ADD_ONE(ctr); RT[0][6] = XOR(ctr, Z); */
    /*     ctr =  ADD_ONE(ctr); RT[0][7] = XOR(ctr, Z); */

    /*     for(i=1; i<8; i++){ //UPDATE_TWEAK */ 
    /*         RT[i][0] = PERMUTE(RT[i-1][0]); */
    /*         RT[i][1] = PERMUTE(RT[i-1][1]); */
    /*         RT[i][2] = PERMUTE(RT[i-1][2]); */
    /*         RT[i][3] = PERMUTE(RT[i-1][3]); */
    /*         RT[i][4] = PERMUTE(RT[i-1][4]); */
    /*         RT[i][5] = PERMUTE(RT[i-1][5]); */
    /*         RT[i][6] = PERMUTE(RT[i-1][6]); */
    /*         RT[i][7] = PERMUTE(RT[i-1][7]); */
    /*     } */
    /*     States[0] = States[1] = States[2] = States[3] = W; */
    /*     States[4] = States[5] = States[6] = States[7] = W; */
    /*     DEOXYS8( States, key, RT ) */

    /*     ctp[index    ] = XOR(States[0], ptp[index    ]); */
    /*     ctp[index + 1] = XOR(States[1], ptp[index + 1]); */
    /*     ctp[index + 2] = XOR(States[2], ptp[index + 2]); */
    /*     ctp[index + 3] = XOR(States[3], ptp[index + 3]); */
    /*     ctp[index + 4] = XOR(States[4], ptp[index + 4]); */
    /*     ctp[index + 5] = XOR(States[5], ptp[index + 5]); */
    /*     ctp[index + 6] = XOR(States[6], ptp[index + 6]); */
    /*     ctp[index + 7] = XOR(States[7], ptp[index + 7]); */

    /*     index += 8; */
    /*     len -= 128; */
    /* } */
    
    /* while(len >= 64) { */
    /*     ctr =  ADD_ONE(ctr); RT[0][0] = XOR(ctr, Z); */
    /*     ctr =  ADD_ONE(ctr); RT[0][1] = XOR(ctr, Z); */
    /*     ctr =  ADD_ONE(ctr); RT[0][2] = XOR(ctr, Z); */
    /*     ctr =  ADD_ONE(ctr); RT[0][3] = XOR(ctr, Z); */

    /*     for(i=1; i<8; i++){ //UPDATE_TWEAK */ 
    /*         RT[i][0] = PERMUTE(RT[i-1][0]); */
    /*         RT[i][1] = PERMUTE(RT[i-1][1]); */
    /*         RT[i][2] = PERMUTE(RT[i-1][2]); */
    /*         RT[i][3] = PERMUTE(RT[i-1][3]); */
    /*     } */
    /*     States[0] = States[1] = States[2] = States[3] = W; */
    /*     DEOXYS4( States, key, RT ) */

    /*     ctp[index    ] = XOR(States[0], ptp[index    ]); */
    /*     ctp[index + 1] = XOR(States[1], ptp[index + 1]); */
    /*     ctp[index + 2] = XOR(States[2], ptp[index + 2]); */
    /*     ctp[index + 3] = XOR(States[3], ptp[index + 3]); */
        
    /*     index += 4; */
    /*     len -= 64; */
    /* } */

    BLOCK S, T, t;
    while (len > 0) {
        ctr = ADD_ONE(ctr); T = XOR(ctr, Z);
        S = W;
        TAES(S, key, T, t);
        ctp[index] = XOR(S, ptp[index]);
        index += 1;
        len -= 16;
    }

}
