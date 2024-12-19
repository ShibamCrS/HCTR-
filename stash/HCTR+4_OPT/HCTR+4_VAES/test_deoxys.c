#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "./include/setup.h"
#include "./include/deoxysbc.h"

void printreg(const void *a, int nrof_byte){
    int i;
    unsigned char *f = (unsigned char *)a;
    for(i=0; i < nrof_byte; i++){
        printf("%02X",(unsigned char) f[i]); //uint8_t c[4+8];
        if(nrof_byte > 16) if((i%16) == 15) printf(" ");
    }
    printf("\n");
}
void test_parallel_VAES(unsigned char *p, unsigned char *t, __m128i *ekey){
    unsigned char pp[64], tt[64];
    for(int i=0; i<16; i++) {
        pp[i]    =  p[i];
        pp[i+16] =  p[i];
        pp[i+32] =  p[i];
        pp[i+48] =  p[i];
        
        tt[i]    =  t[i];
        tt[i+16] =  t[i];
        tt[i+32] =  t[i];
        tt[i+48] =  t[i];
    }
    __m512i state = _mm512_loadu_si512((__m512i*)pp);;
    __m512i tweak = _mm512_loadu_si512((__m512i*)tt);;
    printreg(&state, 64);
    printreg(&tweak, 64);


    __m512i ekey4[DEOXYS_BC_128_256_NUM_ROUND_KEYS];
    for(int i=0; i<=DEOXYS_BC_128_256_NUM_ROUNDS; i++) ekey4[i] = _mm512_broadcast_i32x4(ekey[i]);
    /* for(int i=0; i<=DEOXYS_BC_128_256_NUM_ROUNDS; i++) printreg(&ekey4[i], 64); */
    
    __m512i RT[8];
    RT[0] = tweak;
    for(int i=1; i<8; i++){ //UPDATE_TWEAK
        RT[i] = PERMUTE_512(RT[i-1]);
    }
    DEOXYS( state, ekey4, RT )
    __m128i States[4];
    States[0] = _mm512_extracti32x4_epi32(state, 0);
    States[1] = _mm512_extracti32x4_epi32(state, 1);
    States[2] = _mm512_extracti32x4_epi32(state, 2);
    States[3] = _mm512_extracti32x4_epi32(state, 3);
    printreg(&state, 64);
    printreg(&States[0], 16);
    printreg(&States[1], 16);
    printreg(&States[2], 16);
    printreg(&States[3], 16);
}
int main() {
    ALIGN(16) unsigned char mkey[16] = {0x2B,0x7E,0x15,0x16,0x28,0xAE,0xD2,0xA6,0xAB,0xF7,0x15,0x88,0x09,0xCF,0x4F,0x3C};
    ALIGN(16) unsigned char tweak[16] = {0x1B,0x2E,0x35,0x46,0x58,0x6E,0x72,0x86,0x9B,0xA7,0xB5,0xC8,0xD9,0xEF,0xFF,0x0C};
    ALIGN(16) unsigned char p[16] = {0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x23, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    ALIGN(16) unsigned char c[16];

    ALIGN(16) __m128i ekey  [DEOXYS_BC_128_256_NUM_ROUND_KEYS];
    ALIGN(16) __m128i dkey  [DEOXYS_BC_128_256_NUM_ROUND_KEYS];
    ALIGN(16) __m128i etweak[DEOXYS_BC_128_256_NUM_ROUND_KEYS];
    ALIGN(16) __m128i dtweak[DEOXYS_BC_128_256_NUM_ROUND_KEYS];

    DEOXYS_128_256_setup_key  (mkey,  ekey);
    DEOXYS_128_256_setup_tweak(tweak, etweak);
        
    DEOXYS_128_256_setup_key_decryption(dkey, ekey);
    DEOXYS_128_256_setup_key_decryption(dtweak, etweak);
    
    for(int i=0; i<=DEOXYS_BC_128_256_NUM_ROUNDS; i++) printreg(&ekey[i], 16);
    printf("---------------------------------------\n");
    /* for(int i=0; i<=DEOXYS_BC_128_256_NUM_ROUNDS; i++) printreg(&dkey[i], 16); */
    /* printf("---------------------------------------\n"); */
    for(int i=0; i<=DEOXYS_BC_128_256_NUM_ROUNDS; i++) printreg(&etweak[i], 16);
    printf("---------------------------------------\n");
    /* for(int i=0; i<=DEOXYS_BC_128_256_NUM_ROUNDS; i++) printreg(&dtweak[i], 16); */
    /* printf("---------------------------------------\n"); */

    printreg(p, 16);
    printreg(mkey, 16);
    printreg(tweak, 16);
    DEOXYS_128_256_encrypt(ekey, etweak, p, c);
    printreg(c, 16);
    DEOXYS_128_256_decrypt(dkey, dtweak, c, p);
    printreg(p, 16);

    printf("----------------------------------------------------\n");
    test_parallel_VAES(p, tweak, ekey);
    printf("----------------------------------------------------\n");

    __m128i S, T, t;
    T = _mm_load_si128 ((BLOCK*)tweak);
    S = _mm_load_si128 ((BLOCK*)p);
    TAES(S, ekey, T, t);
    printreg(&S, 16);
    TAESD(S, dkey, T, t);
    printreg(&S, 16);
    printf("----------------------------------------------------\n");
    S = PERMUTE(S);
    printreg(&S, 16);
    S = PERMUTEINV(S);
    printreg(&S, 16);
    T = PERMUTE14(S);
    for(int i=0; i<14; i++) S = PERMUTE(S);
    printreg(&S, 16);
    printreg(&T, 16);
}

