#include "setup512.h"

#define DEOXYS_BC_128_256_NUM_ROUNDS      14
#define DEOXYS_BC_128_256_NUM_ROUND_KEYS  (DEOXYS_BC_128_256_NUM_ROUNDS+1)

// ---------------------------------------------------------
#define H_PERMUTATION _mm_setr_epi8( 1,6,11,12,5,10,15,0,9,14,3,4,13,2,7,8)
#define H_PERMUTATION_INV _mm_setr_epi8(7,0,13,10, 11,4,1,14, 15,8,5,2, 3,12,9,6)
#define H_PERMUTATION_14 _mm_setr_epi8(14,7,12,5,2,11,0,9,6,15,4,13,10,3,8,1)

#define PERMUTE(x)     _mm_shuffle_epi8(x, H_PERMUTATION  )
#define PERMUTEINV(x)  _mm_shuffle_epi8(x, H_PERMUTATION_INV)
#define PERMUTE14(x)   _mm_shuffle_epi8(x, H_PERMUTATION_14)


// ---------------------------------------------------------
/* #define H_PERMUTATION_512 _mm512_set_epi8(8, 7, 2, 13, 4, 3, 14, 9, 0, 15, 10, 5, 12, 11, 6, 1, 8, 7, 2, 13, 4, 3, 14, 9, 0, 15, 10, 5, 12, 11, 6, 1, 8, 7, 2, 13, 4, 3, 14, 9, 0, 15, 10, 5, 12, 11, 6, 1, 8, 7, 2, 13, 4, 3, 14, 9, 0, 15, 10, 5, 12, 11, 6, 1) */

#define H_PERMUTATION_512    _mm512_set_epi8(8, 7, 2, 13,  4, 3, 14, 9,  0, 15, 10, 5,  12, 11, 6, 1,  8, 7, 2, 13,  4, 3, 14, 9,  0, 15, 10, 5,  12, 11, 6, 1,  8, 7, 2, 13,  4, 3, 14, 9,  0, 15, 10, 5,  12, 11, 6, 1,  8, 7, 2, 13,  4, 3, 14, 9,  0, 15, 10, 5,  12, 11, 6, 1)
#define H_PERMUTATION_512_2  _mm512_set_epi8(9, 0, 11, 2,  5, 12, 7, 14,  1, 8, 3, 10,  13, 4, 15, 6,  9, 0, 11, 2,  5, 12, 7, 14,  1, 8, 3, 10,  13, 4, 15, 6,  9, 0, 11, 2,  5, 12, 7, 14,  1, 8, 3, 10,  13, 4, 15, 6,  9, 0, 11, 2,  5, 12, 7, 14,  1, 8, 3, 10,  13, 4, 15, 6)
#define H_PERMUTATION_512_3  _mm512_set_epi8(14, 1, 4, 11,  10, 13, 0, 7,  6, 9, 12, 3,  2, 5, 8, 15,  14, 1, 4, 11,  10, 13, 0, 7,  6, 9, 12, 3,  2, 5, 8, 15,  14, 1, 4, 11,  10, 13, 0, 7,  6, 9, 12, 3,  2, 5, 8, 15,  14, 1, 4, 11,  10, 13, 0, 7,  6, 9, 12, 3,  2, 5, 8, 15)

#define PERMUTE_512(x)      _mm512_shuffle_epi8(x, H_PERMUTATION_512)
#define PERMUTE_512_2(x)    _mm512_shuffle_epi8(x, H_PERMUTATION_512_2)
#define PERMUTE_512_3(x)    _mm512_shuffle_epi8(x, H_PERMUTATION_512_3)

// 0x4e = 01 00 11 10 = swap 64-bit halves
#define swap64_512(x)       _mm512_shuffle_epi32(x, 0x4e)

#define eight_512           _mm512_set_epi64(0, 8, 0, 8, 0, 8, 0, 8)

#define four_512            _mm512_set_epi8(0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 4,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 4,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 4,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 4)
#define four_512_1          _mm512_set_epi8(0, 0, 0, 0,  0, 0, 0, 0,  4, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  4, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  4, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  4, 0, 0, 0,  0, 0, 0, 0)
#define four_512_2          _mm512_set_epi8(0, 4, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 4, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 4, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 4, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0)
#define four_512_3          _mm512_set_epi8(0, 0, 0, 0,  0, 0, 4, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 4, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 4, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 4, 0,  0, 0, 0, 0,  0, 0, 0, 0)
#define four_512_4          _mm512_set_epi8(0, 0, 0, 0,  0, 0, 0, 4,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 4,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 4,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 4,  0, 0, 0, 0,  0, 0, 0, 0)
#define four_512_5          _mm512_set_epi8(4, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  4, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  4, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  4, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0)
#define four_512_6          _mm512_set_epi8(0, 0, 0, 0,  0, 0, 0, 0,  0, 4, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 4, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 4, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 4, 0, 0,  0, 0, 0, 0)
#define four_512_7          _mm512_set_epi8(0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 4, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 4, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 4, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 4, 0)

// ---------------------------------------------------------


/*---------------From SUPERCOP DEOXYSBC---------------------------------------*/
#define ONE_XOR4( s, subkey, tweak )\
    s[0] = _mm512_xor_si512(s[0], _mm512_xor_si512(subkey, tweak[0][0]));\
    s[1] = _mm512_xor_si512(s[1], _mm512_xor_si512(subkey, tweak[1][0]));\
    s[2] = _mm512_xor_si512(s[2], _mm512_xor_si512(subkey, tweak[2][0]));\
    s[3] = _mm512_xor_si512(s[3], _mm512_xor_si512(subkey, tweak[3][0]));\
;
#define ONE_ROUND4( s, subkey, tweak , Round )\
    s[0] = _mm512_aesenc_epi128(s[0], _mm512_xor_si512(subkey, tweak[0][Round]));\
    s[1] = _mm512_aesenc_epi128(s[1], _mm512_xor_si512(subkey, tweak[1][Round]));\
    s[2] = _mm512_aesenc_epi128(s[2], _mm512_xor_si512(subkey, tweak[2][Round]));\
    s[3] = _mm512_aesenc_epi128(s[3], _mm512_xor_si512(subkey, tweak[3][Round]));\
;
#define DEOXYS4( states, subkeys, tweak ) {\
  ONE_XOR4  ( states , subkeys[ 0] , tweak      );\
  ONE_ROUND4( states , subkeys[ 1] , tweak ,  1 );\
  ONE_ROUND4( states , subkeys[ 2] , tweak ,  2 );\
  ONE_ROUND4( states , subkeys[ 3] , tweak ,  3 );\
  ONE_ROUND4( states , subkeys[ 4] , tweak ,  4 );\
  ONE_ROUND4( states , subkeys[ 5] , tweak ,  5 );\
  ONE_ROUND4( states , subkeys[ 6] , tweak ,  6 );\
  ONE_ROUND4( states , subkeys[ 7] , tweak ,  7 );\
  ONE_ROUND4( states , subkeys[ 8] , tweak ,  0 );\
  ONE_ROUND4( states , subkeys[ 9] , tweak ,  1 );\
  ONE_ROUND4( states , subkeys[10] , tweak ,  2 );\
  ONE_ROUND4( states , subkeys[11] , tweak ,  3 );\
  ONE_ROUND4( states , subkeys[12] , tweak ,  4 );\
  ONE_ROUND4( states , subkeys[13] , tweak ,  5 );\
  ONE_ROUND4( states , subkeys[14] , tweak ,  6 );\
}
#define ONE_XOR2( s, subkey, tweak ){ \
    s[0] = _mm512_xor_si512(s[0], _mm512_xor_si512(subkey, tweak[0][0]));\
    s[1] = _mm512_xor_si512(s[1], _mm512_xor_si512(subkey, tweak[1][0]));\
}

#define ONE_ROUND2( s, subkey, tweak , Round ) {\
    s[0] = _mm512_aesenc_epi128(s[0], _mm512_xor_si512(subkey, tweak[0][Round]));\
    s[1] = _mm512_aesenc_epi128(s[1], _mm512_xor_si512(subkey, tweak[1][Round]));\
}

#define DEOXYS2( states, subkeys, tweak ) {\
  ONE_XOR2  ( states , subkeys[ 0] , tweak      );\
  ONE_ROUND2( states , subkeys[ 1] , tweak ,  1 );\
  ONE_ROUND2( states , subkeys[ 2] , tweak ,  2 );\
  ONE_ROUND2( states , subkeys[ 3] , tweak ,  3 );\
  ONE_ROUND2( states , subkeys[ 4] , tweak ,  4 );\
  ONE_ROUND2( states , subkeys[ 5] , tweak ,  5 );\
  ONE_ROUND2( states , subkeys[ 6] , tweak ,  6 );\
  ONE_ROUND2( states , subkeys[ 7] , tweak ,  7 );\
  ONE_ROUND2( states , subkeys[ 8] , tweak ,  0 );\
  ONE_ROUND2( states , subkeys[ 9] , tweak ,  1 );\
  ONE_ROUND2( states , subkeys[10] , tweak ,  2 );\
  ONE_ROUND2( states , subkeys[11] , tweak ,  3 );\
  ONE_ROUND2( states , subkeys[12] , tweak ,  4 );\
  ONE_ROUND2( states , subkeys[13] , tweak ,  5 );\
  ONE_ROUND2( states , subkeys[14] , tweak ,  6 );\
}
#define ONE_XOR( s, subkey, tweak )\
    s = _mm512_xor_si512(s, _mm512_xor_si512(subkey, tweak[0]));\
;
#define ONE_ROUND( s, subkey, tweak , Round )\
    s = _mm512_aesenc_epi128(s, _mm512_xor_si512(subkey, tweak[Round]));\
;
#define DEOXYS( states, subkeys, tweak ) {\
  ONE_XOR  ( states , subkeys[ 0] , tweak      );\
  ONE_ROUND( states , subkeys[ 1] , tweak ,  1 );\
  ONE_ROUND( states , subkeys[ 2] , tweak ,  2 );\
  ONE_ROUND( states , subkeys[ 3] , tweak ,  3 );\
  ONE_ROUND( states , subkeys[ 4] , tweak ,  4 );\
  ONE_ROUND( states , subkeys[ 5] , tweak ,  5 );\
  ONE_ROUND( states , subkeys[ 6] , tweak ,  6 );\
  ONE_ROUND( states , subkeys[ 7] , tweak ,  7 );\
  ONE_ROUND( states , subkeys[ 8] , tweak ,  0 );\
  ONE_ROUND( states , subkeys[ 9] , tweak ,  1 );\
  ONE_ROUND( states , subkeys[10] , tweak ,  2 );\
  ONE_ROUND( states , subkeys[11] , tweak ,  3 );\
  ONE_ROUND( states , subkeys[12] , tweak ,  4 );\
  ONE_ROUND( states , subkeys[13] , tweak ,  5 );\
  ONE_ROUND( states , subkeys[14] , tweak ,  6 );\
}

/* Tweakable AES */
#define TAES( s , subkeys , realtweak, t)\
    t = realtweak;\
    s = XOR( s , XOR( subkeys[ 0] , t ) );t=PERMUTE( t );\
    s = ENC( s , XOR( subkeys[ 1] , t ) );t=PERMUTE( t );\
    s = ENC( s , XOR( subkeys[ 2] , t ) );t=PERMUTE( t );\
    s = ENC( s , XOR( subkeys[ 3] , t ) );t=PERMUTE( t );\
    s = ENC( s , XOR( subkeys[ 4] , t ) );t=PERMUTE( t );\
    s = ENC( s , XOR( subkeys[ 5] , t ) );t=PERMUTE( t );\
    s = ENC( s , XOR( subkeys[ 6] , t ) );t=PERMUTE( t );\
    s = ENC( s , XOR( subkeys[ 7] , t ) );t=PERMUTE( t );\
    s = ENC( s , XOR( subkeys[ 8] , t ) );t=PERMUTE( t );\
    s = ENC( s , XOR( subkeys[ 9] , t ) );t=PERMUTE( t );\
    s = ENC( s , XOR( subkeys[10] , t ) );t=PERMUTE( t );\
    s = ENC( s , XOR( subkeys[11] , t ) );t=PERMUTE( t );\
    s = ENC( s , XOR( subkeys[12] , t ) );t=PERMUTE( t );\
    s = ENC( s , XOR( subkeys[13] , t ) );t=PERMUTE( t );\
    s = ENC( s , XOR( subkeys[14] , t ) );


/* Tweakable AES decryption */
#define TAESD( s , subkeys , realtweak, t)\
    t = realtweak;t=PERMUTE14( t );\
    s = XOR( s , XOR( subkeys[14] , t ) );t=PERMUTEINV( t );\
    s = MCINV(s);\
    s = DEC( s , XOR( subkeys[13] , MCINV(t) ) );t=PERMUTEINV( t );\
    s = DEC( s , XOR( subkeys[12] , MCINV(t) ) );t=PERMUTEINV( t );\
    s = DEC( s , XOR( subkeys[11] , MCINV(t) ) );t=PERMUTEINV( t );\
    s = DEC( s , XOR( subkeys[10] , MCINV(t) ) );t=PERMUTEINV( t );\
    s = DEC( s , XOR( subkeys[ 9] , MCINV(t) ) );t=PERMUTEINV( t );\
    s = DEC( s , XOR( subkeys[ 8] , MCINV(t) ) );t=PERMUTEINV( t );\
    s = DEC( s , XOR( subkeys[ 7] , MCINV(t) ) );t=PERMUTEINV( t );\
    s = DEC( s , XOR( subkeys[ 6] , MCINV(t) ) );t=PERMUTEINV( t );\
    s = DEC( s , XOR( subkeys[ 5] , MCINV(t) ) );t=PERMUTEINV( t );\
    s = DEC( s , XOR( subkeys[ 4] , MCINV(t) ) );t=PERMUTEINV( t );\
    s = DEC( s , XOR( subkeys[ 3] , MCINV(t) ) );t=PERMUTEINV( t );\
    s = DEC( s , XOR( subkeys[ 2] , MCINV(t) ) );t=PERMUTEINV( t );\
    s = DEC( s , XOR( subkeys[ 1] , MCINV(t) ) );t=PERMUTEINV( t );\
    s = DECLAST( s , XOR( subkeys[0] , t ) );


// ---------------------------------------------------------

#define UPDATE_TWEAK_ROUNDS_512_2(tweaks) { \
    tweaks[1] = PERMUTE_512(tweaks[0]);\
    tweaks[2] = PERMUTE_512_2(tweaks[0]);\
    tweaks[3] = PERMUTE_512_3(tweaks[0]);\
    tweaks[4] = swap64_512(tweaks[0]);\
    tweaks[5] = swap64_512(tweaks[1]);\
    tweaks[6] = swap64_512(tweaks[2]);\
    tweaks[7] = swap64_512(tweaks[3]);\
}

// ---------------------------------------------------------

#define UPDATE_TWEAK_ROUNDS_512(tweaks) { \
    tweaks[1] = PERMUTE_512(tweaks[0]);\
    tweaks[2] = PERMUTE_512_2(tweaks[0]);\
    tweaks[3] = PERMUTE_512_3(tweaks[0]);\
}

// ---------------------------------------------------------

#define ONE_ROUND_TWEAK_DEPENDENT_512_2(s, subkey, tweak, tmp, four_permuted){\
    tmp = XOR4(subkey, tweak); \
    s[0] = ENC4(s[0], tmp);\
    s[1] = ENC4(s[1], XOR4(tmp, four_permuted));\
}

// ---------------------------------------------------------

#define ONE_XOR_FIXED_INPUT_512_2(s, tweak, input){\
    s[0] = XOR4(input, tweak); \
    s[1] = XOR4(s[0], four_512); \
}

// ---------------------------------------------------------

#define DEOXYS_FIXED_INPUT_512_2(states, subkeys, tweaks, input, tmp) { \
  ONE_XOR_FIXED_INPUT_512_2(states, tweaks[0], input); \
  ONE_ROUND_TWEAK_DEPENDENT_512_2(states, subkeys[ 1], tweaks[1], tmp, four_512_1); \
  ONE_ROUND_TWEAK_DEPENDENT_512_2(states, subkeys[ 2], tweaks[2], tmp, four_512_2); \
  ONE_ROUND_TWEAK_DEPENDENT_512_2(states, subkeys[ 3], tweaks[3], tmp, four_512_3); \
  ONE_ROUND_TWEAK_DEPENDENT_512_2(states, subkeys[ 4], tweaks[4], tmp, four_512_4); \
  ONE_ROUND_TWEAK_DEPENDENT_512_2(states, subkeys[ 5], tweaks[5], tmp, four_512_5); \
  ONE_ROUND_TWEAK_DEPENDENT_512_2(states, subkeys[ 6], tweaks[6], tmp, four_512_6); \
  ONE_ROUND_TWEAK_DEPENDENT_512_2(states, subkeys[ 7], tweaks[7], tmp, four_512_7); \
  ONE_ROUND_TWEAK_DEPENDENT_512_2(states, subkeys[ 8], tweaks[0], tmp, four_512  ); \
  ONE_ROUND_TWEAK_DEPENDENT_512_2(states, subkeys[ 9], tweaks[1], tmp, four_512_1); \
  ONE_ROUND_TWEAK_DEPENDENT_512_2(states, subkeys[10], tweaks[2], tmp, four_512_2); \
  ONE_ROUND_TWEAK_DEPENDENT_512_2(states, subkeys[11], tweaks[3], tmp, four_512_3); \
  ONE_ROUND_TWEAK_DEPENDENT_512_2(states, subkeys[12], tweaks[4], tmp, four_512_4); \
  ONE_ROUND_TWEAK_DEPENDENT_512_2(states, subkeys[13], tweaks[5], tmp, four_512_5); \
  ONE_ROUND_TWEAK_DEPENDENT_512_2(states, subkeys[14], tweaks[6], tmp, four_512_6); \
}

// ---------------------------------------------------------

#define ONE_ROUND_TWEAK_DEPENDENT_512(s, subkey, tweak){\
    s[0] = ENC4(s[0], XOR4(subkey, tweak));\
}

// ---------------------------------------------------------

#define ONE_XOR_FIXED_INPUT_512(s, tweak, input){\
    s[0] = XOR4(input, tweak); \
}

// ---------------------------------------------------------

#define DEOXYS_FIXED_INPUT_512(states, subkeys, tweaks, input) { \
  ONE_XOR_FIXED_INPUT_512(states, tweaks[0], input); \
  ONE_ROUND_TWEAK_DEPENDENT_512(states, subkeys[ 1], tweaks[1]); \
  ONE_ROUND_TWEAK_DEPENDENT_512(states, subkeys[ 2], tweaks[2]); \
  ONE_ROUND_TWEAK_DEPENDENT_512(states, subkeys[ 3], tweaks[3]); \
  ONE_ROUND_TWEAK_DEPENDENT_512(states, subkeys[ 4], tweaks[4]); \
  ONE_ROUND_TWEAK_DEPENDENT_512(states, subkeys[ 5], tweaks[5]); \
  ONE_ROUND_TWEAK_DEPENDENT_512(states, subkeys[ 6], tweaks[6]); \
  ONE_ROUND_TWEAK_DEPENDENT_512(states, subkeys[ 7], tweaks[7]); \
  ONE_ROUND_TWEAK_DEPENDENT_512(states, subkeys[ 8], tweaks[0]); \
  ONE_ROUND_TWEAK_DEPENDENT_512(states, subkeys[ 9], tweaks[1]); \
  ONE_ROUND_TWEAK_DEPENDENT_512(states, subkeys[10], tweaks[2]); \
  ONE_ROUND_TWEAK_DEPENDENT_512(states, subkeys[11], tweaks[3]); \
  ONE_ROUND_TWEAK_DEPENDENT_512(states, subkeys[12], tweaks[4]); \
  ONE_ROUND_TWEAK_DEPENDENT_512(states, subkeys[13], tweaks[5]); \
  ONE_ROUND_TWEAK_DEPENDENT_512(states, subkeys[14], tweaks[6]); \
}


// ---------------------------------------------------------

#define ONE_XOR_HASH_INPUT_512_2(s, tweak){\
    s[0] = XOR4(s[0], tweak); \
    s[1] = XOR4(s[1], XOR4(tweak, four_512)); \
}

// ---------------------------------------------------------

#define DEOXYS_HASH_INPUT_512_2(states, subkeys, tweaks, tmp) { \
  ONE_XOR_HASH_INPUT_512_2(states, tweaks[0]); \
  ONE_ROUND_TWEAK_DEPENDENT_512_2(states, subkeys[ 1], tweaks[1], tmp, four_512_1); \
  ONE_ROUND_TWEAK_DEPENDENT_512_2(states, subkeys[ 2], tweaks[2], tmp, four_512_2); \
  ONE_ROUND_TWEAK_DEPENDENT_512_2(states, subkeys[ 3], tweaks[3], tmp, four_512_3); \
  ONE_ROUND_TWEAK_DEPENDENT_512_2(states, subkeys[ 4], tweaks[4], tmp, four_512_4); \
  ONE_ROUND_TWEAK_DEPENDENT_512_2(states, subkeys[ 5], tweaks[5], tmp, four_512_5); \
  ONE_ROUND_TWEAK_DEPENDENT_512_2(states, subkeys[ 6], tweaks[6], tmp, four_512_6); \
  ONE_ROUND_TWEAK_DEPENDENT_512_2(states, subkeys[ 7], tweaks[7], tmp, four_512_7); \
  ONE_ROUND_TWEAK_DEPENDENT_512_2(states, subkeys[ 8], tweaks[0], tmp, four_512  ); \
  ONE_ROUND_TWEAK_DEPENDENT_512_2(states, subkeys[ 9], tweaks[1], tmp, four_512_1); \
  ONE_ROUND_TWEAK_DEPENDENT_512_2(states, subkeys[10], tweaks[2], tmp, four_512_2); \
  ONE_ROUND_TWEAK_DEPENDENT_512_2(states, subkeys[11], tweaks[3], tmp, four_512_3); \
  ONE_ROUND_TWEAK_DEPENDENT_512_2(states, subkeys[12], tweaks[4], tmp, four_512_4); \
  ONE_ROUND_TWEAK_DEPENDENT_512_2(states, subkeys[13], tweaks[5], tmp, four_512_5); \
  ONE_ROUND_TWEAK_DEPENDENT_512_2(states, subkeys[14], tweaks[6], tmp, four_512_6); \
}

// ---------------------------------------------------------

#define ONE_XOR_HASH_INPUT_512(s, tweak){\
    s[0] = XOR4(s[0], tweak); \
}

#define DEOXYS_HASH_INPUT_512(states, subkeys, tweaks) { \
  ONE_XOR_HASH_INPUT_512(states, tweaks[0]); \
  ONE_ROUND_TWEAK_DEPENDENT_512(states, subkeys[ 1], tweaks[1]); \
  ONE_ROUND_TWEAK_DEPENDENT_512(states, subkeys[ 2], tweaks[2]); \
  ONE_ROUND_TWEAK_DEPENDENT_512(states, subkeys[ 3], tweaks[3]); \
  ONE_ROUND_TWEAK_DEPENDENT_512(states, subkeys[ 4], tweaks[4]); \
  ONE_ROUND_TWEAK_DEPENDENT_512(states, subkeys[ 5], tweaks[5]); \
  ONE_ROUND_TWEAK_DEPENDENT_512(states, subkeys[ 6], tweaks[6]); \
  ONE_ROUND_TWEAK_DEPENDENT_512(states, subkeys[ 7], tweaks[7]); \
  ONE_ROUND_TWEAK_DEPENDENT_512(states, subkeys[ 8], tweaks[0]); \
  ONE_ROUND_TWEAK_DEPENDENT_512(states, subkeys[ 9], tweaks[1]); \
  ONE_ROUND_TWEAK_DEPENDENT_512(states, subkeys[10], tweaks[2]); \
  ONE_ROUND_TWEAK_DEPENDENT_512(states, subkeys[11], tweaks[3]); \
  ONE_ROUND_TWEAK_DEPENDENT_512(states, subkeys[12], tweaks[4]); \
  ONE_ROUND_TWEAK_DEPENDENT_512(states, subkeys[13], tweaks[5]); \
  ONE_ROUND_TWEAK_DEPENDENT_512(states, subkeys[14], tweaks[6]); \
}

// ---------------------------------------------------------------------

void DEOXYS_128_256_setup_key(const unsigned char *mkey, BLOCK *key);
void DEOXYS_128_256_setup_tweak(const unsigned char *mkey, BLOCK *key);
//Also works for decryption tweak setup
void DEOXYS_128_256_setup_key_decryption(BLOCK *dkey, BLOCK *ekey );
void DEOXYS_128_256_encrypt(const BLOCK *rks, const BLOCK *rts,
                   const unsigned char *in, unsigned char *out);
void DEOXYS_128_256_decrypt(const BLOCK *rks, const BLOCK *rts,
                   const unsigned char *in, unsigned char *out);


