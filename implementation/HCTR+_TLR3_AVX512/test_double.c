#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <immintrin.h>

#include "./include/deoxysbc.h"
#include "./include/setup512.h"
#include "./include/utility.h"

//multiply by 4 over finite field
static inline BLOCK4 mult4by4(BLOCK4 b) {
    const BLOCK4 poly = _mm512_set_epi64(0,135, 0,135, 0,135, 0,135);
    const BLOCK4 mask = _mm512_set_epi64(15,0, 15,0, 15,0, 15,0);
    printreg(&b, 64); 
    BLOCK4 t = _mm512_srli_epi64(b, 60);
    printreg(&t, 64); 
    
    t = _mm512_shuffle_epi32(t, _MM_SHUFFLE(1,0,3,2));
    __m512i mod =  _mm512_clmulepi64_epi128(t, poly, 0x00);
    printreg(&mod, 64); 
    t = _mm512_and_si512(t, mask);
    printreg(&t, 64); 

    b = _mm512_slli_epi64(b, 4);
    b = _mm512_xor_si512(b,t);
    printreg(&b, 64); 
    b = _mm512_xor_si512(b,mod);
    return b;
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
__m128i gf_2_128_double_four_my(__m128i Y, __m128i S[8]) {
    Y = Double(Y);Y = XOR(Y, S[0]); //2Y   +   S0
    Y = Double(Y);Y = XOR(Y, S[1]); //2^2Y +   2S0 +    S1
    Y = Double(Y);Y = XOR(Y, S[2]); //2^3Y + 2^2S0 +   2S1 + S2
    Y = Double(Y);Y = XOR(Y, S[3]); //2^4Y + 2^3S0 + 2^2S1 + 2S2 +S3
    return Y;
}
__m128i gf_2_128_double_eight_my(__m128i Y, __m128i S[8]) {
    Y = Double(Y);Y = XOR(Y, S[0]); //2Y   +   S0
    Y = Double(Y);Y = XOR(Y, S[1]); //2^2Y +   2S0 +    S1
    Y = Double(Y);Y = XOR(Y, S[2]); //2^3Y + 2^2S0 +   2S1
    Y = Double(Y);Y = XOR(Y, S[3]); //2^4Y + 2^3S0 + 2^2S1 +   2S2
    Y = Double(Y);Y = XOR(Y, S[4]); //2^5Y + 2^4S0 + 2^3S1 + 2^2S2 + S3
    Y = Double(Y);Y = XOR(Y, S[5]); //2^6Y + 2^5S0 + 2^4S1 + 2^3S2 + 2^2S3 +  S4
    Y = Double(Y);Y = XOR(Y, S[6]); //2^7Y + 2^6S0 + 2^5S1 + 2^4S2 + 2^3S3 + 2^2S4 + 2S5 + S6
    Y = Double(Y);Y = XOR(Y, S[7]); //2^8Y + 2^7S0 + 2^6S1 + 2^5S2 + 2^4S3 + 2^3S4 + 2^2S5 + 2S6 + S7
    return Y;
}
__m128i gf_2_128_double_16_my(__m128i Y, __m128i S[16]) {
    Y = Double(Y);Y = XOR(Y, S[0]); //2Y     +   S0
    Y = Double(Y);Y = XOR(Y, S[1]); //2^2Y   +   2S0 +    S1
    Y = Double(Y);Y = XOR(Y, S[2]); //2^3Y   + 2^2S0 +   2S1
    Y = Double(Y);Y = XOR(Y, S[3]); //2^4Y   + 2^3S0 + 2^2S1 +   2S2
    Y = Double(Y);Y = XOR(Y, S[4]); //2^5Y   + 2^4S0 + 2^3S1 + 2^2S2 + S3
    Y = Double(Y);Y = XOR(Y, S[5]); //2^6Y   + 2^5S0 + 2^4S1 + 2^3S2 + 2^2S3 +  S4
    Y = Double(Y);Y = XOR(Y, S[6]); //2^7Y   + 2^6S0 + 2^5S1 + 2^4S2 + 2^3S3 + 2^2S4 + 2S5 + S6
    Y = Double(Y);Y = XOR(Y, S[7]); //2^8Y   + 2^7S0 + 2^6S1 + 2^5S2 + 2^4S3 + 2^3S4 + 2^2S5 + 2S6 + S7
    Y = Double(Y);Y = XOR(Y, S[8]); //2^9Y   + ....................................  
    Y = Double(Y);Y = XOR(Y, S[9]); //2^10Y  + .................................... 
    Y = Double(Y);Y = XOR(Y, S[10]); //2^11Y + .................................... 
    Y = Double(Y);Y = XOR(Y, S[11]); //2^12Y + .................................... 
    Y = Double(Y);Y = XOR(Y, S[12]); //2^13Y + .................................... 
    Y = Double(Y);Y = XOR(Y, S[13]); //2^14Y + .................................... 
    Y = Double(Y);Y = XOR(Y, S[14]); //2^15Y + .................................... 
    Y = Double(Y);Y = XOR(Y, S[15]); //2^16Y + .................................... 
    return Y;
}
#define VAL_LEN 256  //16 blocks
void test_parallel_mul() {
    BLOCK RP = REDUCTION_POLYNOMIAL;
    printreg(&RP, 16);
    BLOCK Y = _mm_set_epi8(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
    printreg(&Y, 16);

    uint8_t s[VAL_LEN];
    uint8_t t[VAL_LEN];
    BLOCK s1, t1;
    for (int i=0; i<VAL_LEN; i++) s[i] = 'b'+(i%3);
    BLOCK *S = (BLOCK *)s;
    /* for (int i=0; i<VAL_LEN/16; i++) printreg(&S[i], 16); */ 
    BLOCK Z = ZERO();
    printf("------------------------------------------------------\n");
    Z = gf_2_128_double_four(Y, S);
    printreg(&Z, 16);
    Z = gf_2_128_double_four_my(Y, S);
    printreg(&Z, 16);
    Z = gf_2_128_double_eight(Y, S);
    printreg(&Z, 16);
    Z = gf_2_128_double_eight_my(Y, S);
    printreg(&Z, 16);
    Z = gf_2_128_double_16(Y, S);
    printreg(&Z, 16);
    Z = gf_2_128_double_16_my(Y, S);
    printreg(&Z, 16);

}

#define MUL_LEN 16
void test_mul() {
    ALIGN(16) uint8_t s[MUL_LEN];
    ALIGN(16) uint8_t t[MUL_LEN];
    BLOCK s1, t1;
    BLOCK4 s4, t4;
    for (int i=0; i<MUL_LEN; i++) s[i] = 'c'+ rand();
    printf("------------------------------------------------------\n");
    printreg(s, MUL_LEN);
    gf_double(t, s, MUL_LEN);
    printreg(t, MUL_LEN);
    s1 = *(BLOCK *)s;
    printreg(&s1, MUL_LEN);
    t1 = Double(s1);
    printreg(&t1, MUL_LEN);
    t1 = Double(Double(Double(Double(s1))));
    printreg(&t1, MUL_LEN);

    s4 = _mm512_broadcast_i64x2(s1);
    printreg(&s4, MUL_LEN*4);
    t4 = mult4by4(s4); 
    printreg(&t4, MUL_LEN*4);
}

void test_shuffle() {
    BLOCK4 A, B, C, D;
    A = _mm512_set_epi64(0, 0xF, 0, 0xF, 0, 0xF, 0, 0xF);
    B = _mm512_set_epi64(0, 135, 0, 135, 0, 135, 0, 135);
    C = _mm512_clmulepi64_epi128(A, B, 0x00);
    printf("------------------------------------------------------\n");
    printreg(&A, 64);
    printreg(&B, 64);
    printreg(&C, 64);
    printf("------------------------------------------------------\n");
    A = CTR0123;
    printreg(&A, 64);
    A = ADD4444(A);
    printreg(&A, 64);
     // Input vectors
    A = _mm512_set_epi64(15, 14, 13, 12, 11, 10, 9, 8);
    B = _mm512_set_epi64(31, 30, 29, 28, 27, 26, 25, 24);

    // Shuffle to get C = [B2, B0, A2, A0]
    C = _mm512_shuffle_i64x2(A, B, _MM_SHUFFLE(2, 0, 2, 0));

    // Shuffle to get D = [B3, B1, A3, A1]
    D = _mm512_shuffle_i64x2(A, B, _MM_SHUFFLE(3, 1, 3, 1));
    printreg(&A, 64);
    printreg(&B, 64);
    printreg(&C, 64);
    printreg(&D, 64);
}

int main() {
    ALIGN(16) uint8_t d[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    printreg(d, 16);
    uint8_t m[16];
    gf_double(m, d, 16);
    printreg(m, 16);

    ALIGN(16) BLOCK dd = _mm_load_si128((__m128i *)d);
    printreg(&dd, 16);
    BLOCK mm = Double(dd);
    printreg(&mm, 16);

    printf("------------------Parallel Double------------------\n");
    test_parallel_mul();
    printf("------------------------------------------------------\n");
    test_mul();
    printf("------------------------------------------------------\n");
    test_shuffle();
    printf("------------------------------------------------------\n");
}
