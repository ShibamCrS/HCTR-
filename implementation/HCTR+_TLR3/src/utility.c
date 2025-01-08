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

BLOCK gf_2_128_double_four(BLOCK Y, BLOCK S[4]) {
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

void ctr_mode(const BLOCK *ptp, const BLOCK key[DEOXYS_BC_128_256_NUM_ROUND_KEYS], uint64_t len, BLOCK W, BLOCK Z, BLOCK *ctp) {
    
    unsigned remaining, iters, npbytes, index, i;
    index = 0;
    npbytes = 16*BPI;
    iters = len/npbytes;
    BLOCK RT[8][BPI];
    BLOCK States[BPI];

    BLOCK ctr = ZERO();
    BLOCK S, T, t;
    while (iters) {
        ctr =  ADD_ONE(ctr); RT[0][0] = XOR(ctr, Z);
        ctr =  ADD_ONE(ctr); RT[0][1] = XOR(ctr, Z);
        ctr =  ADD_ONE(ctr); RT[0][2] = XOR(ctr, Z);
        ctr =  ADD_ONE(ctr); RT[0][3] = XOR(ctr, Z);

        for(i=1; i<8; i++){ //UPDATE_TWEAK 
            RT[i][0] = PERMUTE(RT[i-1][0]);
            RT[i][1] = PERMUTE(RT[i-1][1]);
            RT[i][2] = PERMUTE(RT[i-1][2]);
            RT[i][3] = PERMUTE(RT[i-1][3]);
        }
        States[0] = States[1] = States[2] = States[3] = W;
        DEOXYS( States, key, RT )

        ctp[index    ] = XOR(States[0], ptp[index    ]);
        ctp[index + 1] = XOR(States[1], ptp[index + 1]);
        ctp[index + 2] = XOR(States[2], ptp[index + 2]);
        ctp[index + 3] = XOR(States[3], ptp[index + 3]);

        index += BPI;
        --iters;
    }
    remaining = len % npbytes;
    while (remaining > 0) {
        ctr = ADD_ONE(ctr); T = XOR(ctr, Z);
        S = W;
        TAES(S, key, T, t);
        ctp[index] = XOR(S, ptp[index]);
        index += 1;
        remaining -= 16;
    }

}
