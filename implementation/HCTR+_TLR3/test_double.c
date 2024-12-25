#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "./include/setup.h"

void printreg(const void *a, int nrof_byte){
    int i;
    unsigned char *f = (unsigned char *)a;
    for(i=0; i < nrof_byte; i++){
        printf("%02X ",(unsigned char) f[i]); //uint8_t c[4+8];
    }
    printf("\n");
}
//multiply by 2 over finite field
static inline BLOCK increment(BLOCK b) {
    const __m128i mask = _mm_set_epi32(135,1,1,1);
    __m128i t = _mm_srai_epi32(b, 31);
    t = _mm_and_si128(t, mask);
    t = _mm_shuffle_epi32(t, _MM_SHUFFLE(2,1,0,3));
    b = _mm_slli_epi32(b, 1);
    return _mm_xor_si128(b,t);
}
void gf_double(uint8_t* out,
               const uint8_t* in,
               const size_t num_bytes) {
    for (size_t i = 1; i < num_bytes; ++i) {
        out[i] = ((in[i] << 1) & 0xfe)
            | ((in[i-1] >> 7) & 0x01);
    }

    const uint8_t msb = (in[num_bytes-1] >> 7) & 0x01;
    out[0] = (in[0] << 1) & 0xfe;
    out[0] ^= msb * 0x87;
}
#define REDUCTION_POLYNOMIAL  _mm_set_epi32(0, 0, 0, 135)
#define accumulate_eight(x, y) { \
    x[0] = XOR(x[0], x[1]); \
    x[2] = XOR(x[2], x[3]); \
    x[4] = XOR(x[4], x[5]); \
    x[6] = XOR(x[6], x[7]); \
    x[0] = XOR(x[0], x[2]); \
    x[4] = XOR(x[4], x[6]); \
    y    = XOR(x[0], x[4]); \
}
// ---------------------------------------------------------------------
__m128i gf_2_128_double_eight(__m128i hash, __m128i x[8]) {
    __m128i tmp[8];
    tmp[0] = _mm_srli_epi64(hash, 56);
    tmp[1] = _mm_srli_epi64(x[0], 57);
    tmp[2] = _mm_srli_epi64(x[1], 58);
    tmp[3] = _mm_srli_epi64(x[2], 59);
    tmp[4] = _mm_srli_epi64(x[3], 60);
    tmp[5] = _mm_srli_epi64(x[4], 61);
    tmp[6] = _mm_srli_epi64(x[5], 62);
    tmp[7] = _mm_srli_epi64(x[6], 63);

    __m128i sum;
    accumulate_eight(tmp, sum);

    // ---------------------------------------------------------------------
    // sum = sum_high || sum_low
    // We have to take sum_high * 135 and XOR it to our XOR sum to have the
    // Reduction term. The 0x01 indicates that sum_high is used.
    // ---------------------------------------------------------------------

    __m128i mod =  _mm_clmulepi64_si128(sum, REDUCTION_POLYNOMIAL, 0x01);

    // Move sum_low to the upper 64-bit half
    __m128i sum_low = _mm_bslli_si128(sum, 8);

    tmp[0] = _mm_slli_epi64(hash, 8);
    tmp[1] = _mm_slli_epi64(x[0], 7);
    tmp[2] = _mm_slli_epi64(x[1], 6);
    tmp[3] = _mm_slli_epi64(x[2], 5);
    tmp[4] = _mm_slli_epi64(x[3], 4);
    tmp[5] = _mm_slli_epi64(x[4], 3);
    tmp[6] = _mm_slli_epi64(x[5], 2);
    tmp[7] = _mm_slli_epi64(x[6], 1);

    accumulate_eight(tmp, sum);
    sum = XOR(sum, sum_low);
    sum = XOR(sum, mod);
    sum = XOR(sum, x[7]);
    return sum;
}
__m128i gf_2_128_double_eight_my(__m128i Y, __m128i S[8]) {
    Y = increment(Y);Y = XOR(Y, S[0]); //2Y   +   S0
    Y = increment(Y);Y = XOR(Y, S[1]); //2^2Y +   2S0 +    S1
    Y = increment(Y);Y = XOR(Y, S[2]); //2^3Y + 2^2S0 +   2S1
    Y = increment(Y);Y = XOR(Y, S[3]); //2^4Y + 2^3S0 + 2^2S1 +   2S2
    Y = increment(Y);Y = XOR(Y, S[4]); //2^5Y + 2^4S0 + 2^3S1 + 2^2S2 + S3
    Y = increment(Y);Y = XOR(Y, S[5]); //2^6Y + 2^5S0 + 2^4S1 + 2^3S2 + 2^2S3 +  S4
    Y = increment(Y);Y = XOR(Y, S[6]); //2^7Y + 2^6S0 + 2^5S1 + 2^4S2 + 2^3S3 + 2^2S4 + 2S5 + S6
    Y = increment(Y);Y = XOR(Y, S[7]); //2^8Y + 2^7S0 + 2^6S1 + 2^5S2 + 2^4S3 + 2^3S4 + 2^2S5 + 2S6 + S7
    return Y;
}
#define accumulate_four(x, y) { \
    x[0] = XOR(x[0], x[1]); \
    x[2] = XOR(x[2], x[3]); \
    y    = XOR(x[0], x[2]); \
}
__m128i gf_2_128_double_four(__m128i Y, __m128i S[4]) {
    __m128i tmp[4];
    tmp[0] = _mm_srli_epi64(Y   , 60);
    tmp[1] = _mm_srli_epi64(S[0], 61);
    tmp[2] = _mm_srli_epi64(S[1], 62);
    tmp[3] = _mm_srli_epi64(S[2], 63);

    __m128i sum;
    accumulate_four(tmp, sum);

    // ---------------------------------------------------------------------
    // sum = sum_high || sum_low
    // We have to take sum_high * 135 and XOR it to our XOR sum to have the
    // Reduction term. The 0x01 indicates that sum_high is used.
    // ---------------------------------------------------------------------

    __m128i mod =  _mm_clmulepi64_si128(sum, REDUCTION_POLYNOMIAL, 0x01);

    // Move sum_low to the upper 64-bit half
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

#define accumulate_five(x, y) { \
    x[0] = XOR(x[0], x[1]); \
    x[2] = XOR(x[2], x[3]); \
    x[0] = XOR(x[0], x[4]); \
    y    = XOR(x[0], x[2]); \
}
__m128i gf_2_128_double_five(__m128i Y, __m128i S[4]) {
    __m128i tmp[5];
    tmp[0] = _mm_srli_epi64(Y   , 60);
    tmp[1] = _mm_srli_epi64(S[0], 60);
    tmp[2] = _mm_srli_epi64(S[1], 61);
    tmp[3] = _mm_srli_epi64(S[2], 62);
    tmp[4] = _mm_srli_epi64(S[3], 63);

    __m128i sum;
    accumulate_five(tmp, sum);
    __m128i mod =  _mm_clmulepi64_si128(sum, REDUCTION_POLYNOMIAL, 0x01);

    // Move sum_low to the upper 64-bit half
    __m128i sum_low = _mm_bslli_si128(sum, 8);

    tmp[0] = _mm_slli_epi64(Y,    4);
    tmp[1] = _mm_slli_epi64(S[0], 4);
    tmp[2] = _mm_slli_epi64(S[1], 3);
    tmp[3] = _mm_slli_epi64(S[2], 2);
    tmp[4] = _mm_slli_epi64(S[3], 1);

    accumulate_five(tmp, sum);
    sum = XOR(sum, sum_low);
    sum = XOR(sum, mod);
    return sum;
}
__m128i gf_2_128_double_five_easy(__m128i Y, __m128i S[4]) {
    __m128i tmp[4];
    Y = XOR(Y, S[0]);
    tmp[0] = _mm_srli_epi64(Y   , 60);
    tmp[1] = _mm_srli_epi64(S[1], 61);
    tmp[2] = _mm_srli_epi64(S[2], 62);
    tmp[3] = _mm_srli_epi64(S[3], 63);

    __m128i sum;
    accumulate_four(tmp, sum);
    __m128i mod =  _mm_clmulepi64_si128(sum, REDUCTION_POLYNOMIAL, 0x01);

    // Move sum_low to the upper 64-bit half
    __m128i sum_low = _mm_bslli_si128(sum, 8);

    tmp[0] = _mm_slli_epi64(Y, 4);
    tmp[1] = _mm_slli_epi64(S[1], 3);
    tmp[2] = _mm_slli_epi64(S[2], 2);
    tmp[3] = _mm_slli_epi64(S[3], 1);

    accumulate_four(tmp, sum);
    sum = XOR(sum, sum_low);
    sum = XOR(sum, mod);
    return sum;
}
__m128i gf_2_128_double_four_my(__m128i Y, __m128i S[8]) {
    Y = increment(Y);Y = XOR(Y, S[0]); //2Y   +   S0
    Y = increment(Y);Y = XOR(Y, S[1]); //2^2Y +   2S0 +    S1
    Y = increment(Y);Y = XOR(Y, S[2]); //2^3Y + 2^2S0 +   2S1 + S2
    Y = increment(Y);Y = XOR(Y, S[3]); //2^4Y + 2^3S0 + 2^2S1 + 2S2 +S3
    return Y;
}
__m128i gf_2_128_double_five_my(__m128i Y, __m128i S[8]) {
    Y = XOR(Y, S[0]); Y = increment(Y); //2(Y + S0)
    Y = XOR(Y, S[1]); Y = increment(Y); //2^2Y + 2^2S0 + 2S1
    Y = XOR(Y, S[2]); Y = increment(Y); //2^3Y + 2^3S0 + 2^2S1 + 2S2
    Y = XOR(Y, S[3]); Y = increment(Y); //2^4Y + 2^4S0 + 2^3S1 + 2^2S2 + 2S3
    return Y;
}
#define VAL_LEN 128  //8 blocks
void test_parallel_mul() {
    ALIGN(16) BLOCK RP = REDUCTION_POLYNOMIAL;
    printreg(&RP, 16);

    ALIGN(16) BLOCK Y = _mm_set_epi8(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
    printreg(&Y, 16);

    ALIGN(16) uint8_t s[VAL_LEN];
    for (int i=0; i<VAL_LEN; i++) s[i] = 'a'+(i%3);
    BLOCK *S = (BLOCK *)s;
    for (int i=0; i<VAL_LEN/16; i++) printreg(&S[i], 16); 
    BLOCK Z = ZERO();
    Z = gf_2_128_double_eight(Y, S);
    printreg(&Z, 16);
    Z = gf_2_128_double_eight_my(Y, S);
    printreg(&Z, 16);
    printf("------------------------------------------------------\n");
    Z = gf_2_128_double_four(Y, S);
    printreg(&Z, 16);
    Z = gf_2_128_double_four_my(Y, S);
    printreg(&Z, 16);
    Z = gf_2_128_double_five(Y, S);
    printreg(&Z, 16);
    Z = gf_2_128_double_five_easy(Y, S);
    printreg(&Z, 16);
    Z = gf_2_128_double_five_my(Y, S);
    printreg(&Z, 16);
    
}
int main() {
    ALIGN(16) uint8_t d[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    printreg(d, 16);
    uint8_t m[16];
    gf_double(m, d, 16);
    printreg(m, 16);

    ALIGN(16) BLOCK dd = _mm_load_si128((__m128i *)d);
    printreg(&dd, 16);
    BLOCK mm = increment(dd);
    printreg(&mm, 16);

    printf("------------------------------------------------------\n");
    test_parallel_mul();
}
