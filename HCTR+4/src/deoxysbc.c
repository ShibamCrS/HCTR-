#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include "../include/setup.h"
#include "../include/deoxysbc.h"

static const unsigned char RCON[17] = { 0x2f, 0x5e, 0xbc, 0x63, 0xc6, 0x97, 0x35, 0x6a, 0xd4, 0xb3, 0x7d, 0xfa, 0xef, 0xc5, 0x91, 0x39, 0x72} ;
/* ----------------LFSR--------------------------------------*/
#define lfsr_two_generic(x, y, tmp, r1, r2, mask) {\
    tmp = _mm_xor_si128(x, _mm_slli_epi16(x, 2));\
    y = _mm_xor_si128(_mm_and_si128(mask, _mm_srli_epi16(tmp, r1)), \
             _mm_andnot_si128(mask, _mm_slli_epi16(x, r2)));\
}
#define lfsr_two(x, y) {lfsr_two_generic(x, y, y, 7, 1, _mm_set1_epi8(0x01));}

static void add_round_constants(BLOCK* round_keys, const size_t num_rounds) {
    for (size_t i = 0; i <= num_rounds; ++i) {
        const BLOCK rcon = _mm_setr_epi8(1, 2, 4, 8, RCON[i], RCON[i], RCON[i], RCON[i], 0, 0, 0, 0, 0, 0, 0, 0);
        round_keys[i] = _mm_xor_si128(round_keys[i], rcon);
    }
}
void DEOXYS_128_256_setup_key(const unsigned char *mkey, BLOCK *key) {
    /* BLOCK *round_keys = key; */
    key[0] = _mm_loadu_si128((BLOCK*)mkey);
    for (size_t i = 0; i < DEOXYS_BC_128_256_NUM_ROUNDS; ++i) {
        lfsr_two(key[i], key[i+1]);
        key[i+1] = PERMUTE(key[i + 1]);
    }
    add_round_constants(key, DEOXYS_BC_128_256_NUM_ROUNDS);
}
void DEOXYS_128_256_setup_tweak(const unsigned char *mkey, BLOCK *key) {
    key[0] = _mm_loadu_si128((BLOCK*)mkey);
    for (size_t i = 0; i < DEOXYS_BC_128_256_NUM_ROUNDS; ++i) {
        key[i+1] = PERMUTE(key[i]);
    }
}

//Also works for decryption tweak setup
void DEOXYS_128_256_setup_key_decryption(BLOCK *dkey, BLOCK *ekey ) {
    dkey[0] = ekey[0];
    for (size_t i = 1; i < DEOXYS_BC_128_256_NUM_ROUNDS; ++i) {
        dkey[i] = _mm_aesimc_si128(ekey[i]);
    }
    dkey[DEOXYS_BC_128_256_NUM_ROUNDS] = ekey[DEOXYS_BC_128_256_NUM_ROUNDS];
}

void DEOXYS_128_256_encrypt(const BLOCK *rks, const BLOCK *rts,
                   const unsigned char *in, unsigned char *out) { 
    int j;
    BLOCK state = _mm_load_si128 ((BLOCK*)in);
    state = _mm_xor_si128(state, _mm_xor_si128(rks[0], rts[0]));
    
    for (j=1; j<=DEOXYS_BC_128_256_NUM_ROUNDS; ++j) { 
        state = _mm_aesenc_si128 (state, _mm_xor_si128(rks[j], rts[j]));
    }

    _mm_store_si128 ((BLOCK*)out, state);
}
void DEOXYS_128_256_decrypt(const BLOCK *rks, const BLOCK *rts,
                   const unsigned char *in, unsigned char *out) { 
    int j;
    BLOCK state = _mm_load_si128 ((BLOCK*)in);
    state = _mm_xor_si128(state, _mm_xor_si128(rks[DEOXYS_BC_128_256_NUM_ROUNDS], rts[DEOXYS_BC_128_256_NUM_ROUNDS]));
    state = _mm_aesimc_si128(state);
    
    for (j=DEOXYS_BC_128_256_NUM_ROUNDS-1; j> 0; --j)  
        state = _mm_aesdec_si128(state, _mm_xor_si128(rks[j], rts[j]));

    state = _mm_aesdeclast_si128(state, _mm_xor_si128(rks[0], rts[0]));
    
    _mm_store_si128 ((BLOCK*)out, state);
}
