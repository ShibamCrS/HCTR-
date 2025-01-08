#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "./include/setup.h"
#include "./include/deoxysbc.h"
#include "./include/utility.h"

void test_parallel_4(unsigned char *p, unsigned char *t, __m128i *ekey){
    /* for(int i=0; i<=DEOXYS_BC_128_256_NUM_ROUNDS; i++) printreg(&ekey[i], 16); */
    __m128i state = _mm_load_si128((__m128i*)p);
    __m128i tweak = _mm_load_si128((__m128i*)t);
    /* printreg(&state, 16); */
    /* printreg(&tweak, 16); */

    __m128i RT[8][4];
    __m128i States[4];
    RT[0][0] = tweak; States[0] = state;
    RT[0][1] = tweak; States[1] = state;
    RT[0][2] = tweak; States[2] = state;
    RT[0][3] = tweak; States[3] = state;

    for(int i=1; i<8; i++){ //UPDATE_TWEAK
        RT[i][0] = PERMUTE(RT[i-1][0]);
        RT[i][1] = PERMUTE(RT[i-1][1]);
        RT[i][2] = PERMUTE(RT[i-1][2]);
        RT[i][3] = PERMUTE(RT[i-1][3]);
    }
    DEOXYS( States, ekey, RT )
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
    for(int i=0; i<=DEOXYS_BC_128_256_NUM_ROUNDS; i++) printreg(&dkey[i], 16);
    printf("---------------------------------------\n");
    for(int i=0; i<=DEOXYS_BC_128_256_NUM_ROUNDS; i++) printreg(&etweak[i], 16);
    printf("---------------------------------------\n");
    for(int i=0; i<=DEOXYS_BC_128_256_NUM_ROUNDS; i++) printreg(&dtweak[i], 16);
    printf("---------------------------------------\n");

    printreg(p, 16);
    printreg(mkey, 16);
    printreg(tweak, 16);
    DEOXYS_128_256_encrypt(ekey, etweak, p, c);
    printreg(c, 16);
    DEOXYS_128_256_decrypt(dkey, dtweak, c, p);
    printreg(p, 16);

    printf("---------------Test Parallel Four---------------------\n");
    test_parallel_4(p, tweak, ekey);
    printf("----------------------------------------------------\n");

    __m128i S, T, t;
    T = _mm_load_si128 ((BLOCK*)tweak);
    S = _mm_load_si128 ((BLOCK*)p);
    printreg(&S, 16);
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

