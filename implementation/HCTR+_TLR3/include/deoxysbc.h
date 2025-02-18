/**
 * @author Anonymized
 * @date   2025-01
 */

// ---------------------------------------------------------

#ifndef _DEOXYS_BC_H
#define _DEOXYS_BC_H

// ---------------------------------------------------------
// Includes
// ---------------------------------------------------------

#include "setup.h"

// ---------------------------------------------------------
// Constants
// ---------------------------------------------------------

#define DEOXYS_BC_128_256_NUM_ROUNDS      14
#define DEOXYS_BC_128_256_NUM_ROUND_KEYS  (DEOXYS_BC_128_256_NUM_ROUNDS+1)

#define H_PERMUTATION     _mm_setr_epi8(1,6,11,12, 5,10,15,0, 9,14,3,4, 13,2,7,8)
#define H_PERMUTATION_INV _mm_setr_epi8(7,0,13,10, 11,4,1,14, 15,8,5,2, 3,12,9,6)
#define H_PERMUTATION_14  _mm_setr_epi8(14,7,12,5, 2,11,0,9, 6,15,4,13, 10,3,8,1)

#define H_PERMUTATION_AVX _mm256_setr_epi8(1,6,11,12, 5,10,15,0, 9,14,3,4, 13,2,7,8, 1,6,11,12, 5,10,15,0, 9,14,3,4, 13,2,7,8)

#define PERMUTE(x)        _mm_shuffle_epi8(x, H_PERMUTATION  )
#define PERMUTEINV(x)     _mm_shuffle_epi8(x, H_PERMUTATION_INV)
#define PERMUTE14(x)      _mm_shuffle_epi8(x, H_PERMUTATION_14)

#define PERMUTE2(x)        _mm_shuffle_epi8(x, _mm_setr_epi8(6, 15, 4, 13,  10, 3, 8, 1,  14, 7, 12, 5,  2, 11, 0, 9))
#define PERMUTE3(x)        _mm_shuffle_epi8(x, _mm_setr_epi8(15, 8, 5, 2,  3, 12, 9, 6,  7, 0, 13, 10,  11, 4, 1, 14))
#define PERMUTE4(x)        _mm_shuffle_epi8(x, _mm_setr_epi8(8, 9, 10, 11,  12, 13, 14, 15,  0, 1, 2, 3,  4, 5, 6, 7))
#define PERMUTE5(x)        _mm_shuffle_epi8(x, _mm_setr_epi8(9, 14, 3, 4,  13, 2, 7, 8,  1, 6, 11, 12,  5, 10, 15, 0))
#define PERMUTE6(x)        _mm_shuffle_epi8(x, _mm_setr_epi8(14, 7, 12, 5,  2, 11, 0, 9,  6, 15, 4, 13,  10, 3, 8, 1))
#define PERMUTE7(x)        _mm_shuffle_epi8(x, _mm_setr_epi8(7, 0, 13, 10,  11, 4, 1, 14,  15, 8, 5, 2,  3, 12, 9, 6))

#define one_be_1           set8r(0, 0, 0, 0,  0, 0, 0, 1,  0, 0, 0, 0,  0, 0, 0, 0)
#define two_be_1           set8r(0, 0, 0, 0,  0, 0, 0, 2,  0, 0, 0, 0,  0, 0, 0, 0)
#define three_be_1         set8r(0, 0, 0, 0,  0, 0, 0, 3,  0, 0, 0, 0,  0, 0, 0, 0)
#define four_be_1          set8r(0, 0, 0, 0,  0, 0, 0, 4,  0, 0, 0, 0,  0, 0, 0, 0)
#define five_be_1          set8r(0, 0, 0, 0,  0, 0, 0, 5,  0, 0, 0, 0,  0, 0, 0, 0)
#define six_be_1           set8r(0, 0, 0, 0,  0, 0, 0, 6,  0, 0, 0, 0,  0, 0, 0, 0)
#define seven_be_1         set8r(0, 0, 0, 0,  0, 0, 0, 7,  0, 0, 0, 0,  0, 0, 0, 0)
#define eight_be_1         set8r(0, 0, 0, 0,  0, 0, 0, 8,  0, 0, 0, 0,  0, 0, 0, 0)

#define one_be_2           set8r(0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 1, 0)
#define two_be_2           set8r(0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 2, 0)
#define three_be_2         set8r(0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 3, 0)
#define four_be_2          set8r(0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 4, 0)
#define five_be_2          set8r(0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 5, 0)
#define six_be_2           set8r(0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 6, 0)
#define seven_be_2         set8r(0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 7, 0)
#define eight_be_2         set8r(0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 8, 0)

#define one_be_3           set8r(0, 0, 0, 0,  0, 0, 0, 0,  0, 1, 0, 0,  0, 0, 0, 0)
#define two_be_3           set8r(0, 0, 0, 0,  0, 0, 0, 0,  0, 2, 0, 0,  0, 0, 0, 0)
#define three_be_3         set8r(0, 0, 0, 0,  0, 0, 0, 0,  0, 3, 0, 0,  0, 0, 0, 0)
#define four_be_3          set8r(0, 0, 0, 0,  0, 0, 0, 0,  0, 4, 0, 0,  0, 0, 0, 0)
#define five_be_3          set8r(0, 0, 0, 0,  0, 0, 0, 0,  0, 5, 0, 0,  0, 0, 0, 0)
#define six_be_3           set8r(0, 0, 0, 0,  0, 0, 0, 0,  0, 6, 0, 0,  0, 0, 0, 0)
#define seven_be_3         set8r(0, 0, 0, 0,  0, 0, 0, 0,  0, 7, 0, 0,  0, 0, 0, 0)
#define eight_be_3         set8r(0, 0, 0, 0,  0, 0, 0, 0,  0, 8, 0, 0,  0, 0, 0, 0)

#define one_be_4           set8r(0, 0, 0, 0,  0, 0, 0, 0,  1, 0, 0, 0,  0, 0, 0, 0)
#define two_be_4           set8r(0, 0, 0, 0,  0, 0, 0, 0,  2, 0, 0, 0,  0, 0, 0, 0)
#define three_be_4         set8r(0, 0, 0, 0,  0, 0, 0, 0,  3, 0, 0, 0,  0, 0, 0, 0)
#define four_be_4          set8r(0, 0, 0, 0,  0, 0, 0, 0,  4, 0, 0, 0,  0, 0, 0, 0)
#define five_be_4          set8r(0, 0, 0, 0,  0, 0, 0, 0,  5, 0, 0, 0,  0, 0, 0, 0)
#define six_be_4           set8r(0, 0, 0, 0,  0, 0, 0, 0,  6, 0, 0, 0,  0, 0, 0, 0)
#define seven_be_4         set8r(0, 0, 0, 0,  0, 0, 0, 0,  7, 0, 0, 0,  0, 0, 0, 0)
#define eight_be_4         set8r(0, 0, 0, 0,  0, 0, 0, 0,  8, 0, 0, 0,  0, 0, 0, 0)

#define one_be_5           set8r(0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1)
#define two_be_5           set8r(0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 2)
#define three_be_5         set8r(0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 3)
#define four_be_5          set8r(0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 4)
#define five_be_5          set8r(0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 5)
#define six_be_5           set8r(0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 6)
#define seven_be_5         set8r(0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 7)
#define eight_be_5         set8r(0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 8)

#define one_be_6           set8r(0, 0, 0, 0,  0, 0, 1, 0,  0, 0, 0, 0,  0, 0, 0, 0)
#define two_be_6           set8r(0, 0, 0, 0,  0, 0, 2, 0,  0, 0, 0, 0,  0, 0, 0, 0)
#define three_be_6         set8r(0, 0, 0, 0,  0, 0, 3, 0,  0, 0, 0, 0,  0, 0, 0, 0)
#define four_be_6          set8r(0, 0, 0, 0,  0, 0, 4, 0,  0, 0, 0, 0,  0, 0, 0, 0)
#define five_be_6          set8r(0, 0, 0, 0,  0, 0, 5, 0,  0, 0, 0, 0,  0, 0, 0, 0)
#define six_be_6           set8r(0, 0, 0, 0,  0, 0, 6, 0,  0, 0, 0, 0,  0, 0, 0, 0)
#define seven_be_6         set8r(0, 0, 0, 0,  0, 0, 7, 0,  0, 0, 0, 0,  0, 0, 0, 0)
#define eight_be_6         set8r(0, 0, 0, 0,  0, 0, 8, 0,  0, 0, 0, 0,  0, 0, 0, 0)

#define one_be_7           set8r(0, 1, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0)
#define two_be_7           set8r(0, 2, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0)
#define three_be_7         set8r(0, 3, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0)
#define four_be_7          set8r(0, 4, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0)
#define five_be_7          set8r(0, 5, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0)
#define six_be_7           set8r(0, 6, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0)
#define seven_be_7         set8r(0, 7, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0)
#define eight_be_7         set8r(0, 8, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0)

// ---------------------------------------------------------
// Macros
// ---------------------------------------------------------

/** 
 * From SUPERCOP DEOXYSBC 
 */
#define UPDATE_TWEAK8(tweak, I){ \
    tweak[I][0] = PERMUTE(tweak[I-1][0]);\
    tweak[I][1] = PERMUTE(tweak[I-1][1]);\
    tweak[I][2] = PERMUTE(tweak[I-1][2]);\
    tweak[I][3] = PERMUTE(tweak[I-1][3]);\
    tweak[I][4] = PERMUTE(tweak[I-1][4]);\
    tweak[I][5] = PERMUTE(tweak[I-1][5]);\
    tweak[I][6] = PERMUTE(tweak[I-1][6]);\
    tweak[I][7] = PERMUTE(tweak[I-1][7]);\
}

// ---------------------------------------------------------

#define UPDATE_TWEAK_ROUNDS8(tweaks) { \
    tweaks[1] = PERMUTE(tweaks[0]);\
    tweaks[2] = PERMUTE2(tweaks[0]);\
    tweaks[3] = PERMUTE3(tweaks[0]);\
    tweaks[4] = swap64(tweaks[0]);\
    tweaks[5] = swap64(tweaks[1]);\
    tweaks[6] = swap64(tweaks[2]);\
    tweaks[7] = swap64(tweaks[3]);\
}

// ---------------------------------------------------------

#define UPDATE_TWEAK_ROUNDS4(tweaks) { \
    tweaks[1] = PERMUTE(tweaks[0]);\
    tweaks[2] = PERMUTE2(tweaks[0]);\
    tweaks[3] = PERMUTE3(tweaks[0]);\
}

// ---------------------------------------------------------

#define SWAP_TWEAK8(tweak, I){ \
    tweak[I][0] = swap64(tweak[I-4][0]);\
    tweak[I][1] = swap64(tweak[I-4][1]);\
    tweak[I][2] = swap64(tweak[I-4][2]);\
    tweak[I][3] = swap64(tweak[I-4][3]);\
    tweak[I][4] = swap64(tweak[I-4][4]);\
    tweak[I][5] = swap64(tweak[I-4][5]);\
    tweak[I][6] = swap64(tweak[I-4][6]);\
    tweak[I][7] = swap64(tweak[I-4][7]);\
}

// ---------------------------------------------------------

#define ADD_EIGHT_PERMUTED(tweak, I, summand) { \
    tweak[I][0] = vadd(tweak[I][0], summand);\
    tweak[I][1] = vadd(tweak[I][1], summand);\
    tweak[I][2] = vadd(tweak[I][2], summand);\
    tweak[I][3] = vadd(tweak[I][3], summand);\
    tweak[I][4] = vadd(tweak[I][4], summand);\
    tweak[I][5] = vadd(tweak[I][5], summand);\
    tweak[I][6] = vadd(tweak[I][6], summand);\
    tweak[I][7] = vadd(tweak[I][7], summand);\
}

// ---------------------------------------------------------

#define ONE_XOR8( s, subkey, tweak ){\
    s[0] = XOR( s[0] , XOR(subkey , tweak[0][0]) );\
    s[1] = XOR( s[1] , XOR(subkey , tweak[0][1]) );\
    s[2] = XOR( s[2] , XOR(subkey , tweak[0][2]) );\
    s[3] = XOR( s[3] , XOR(subkey , tweak[0][3]) ); \
    s[4] = XOR( s[4] , XOR(subkey , tweak[0][4]) ); \
    s[5] = XOR( s[5] , XOR(subkey , tweak[0][5]) ); \
    s[6] = XOR( s[6] , XOR(subkey , tweak[0][6]) ); \
    s[7] = XOR( s[7] , XOR(subkey , tweak[0][7]) ); \
}

// ---------------------------------------------------------

#define ONE_ROUND8( s, subkey, tweak , Round ){\
    s[0] = ENC( s[0] , XOR(subkey, tweak[Round][0] ) );\
    s[1] = ENC( s[1] , XOR(subkey, tweak[Round][1] ) );\
    s[2] = ENC( s[2] , XOR(subkey, tweak[Round][2] ) );\
    s[3] = ENC( s[3] , XOR(subkey, tweak[Round][3] ) );\
    s[4] = ENC( s[4] , XOR(subkey, tweak[Round][4] ) );\
    s[5] = ENC( s[5] , XOR(subkey, tweak[Round][5] ) );\
    s[6] = ENC( s[6] , XOR(subkey, tweak[Round][6] ) );\
    s[7] = ENC( s[7] , XOR(subkey, tweak[Round][7] ) );\
}

// ---------------------------------------------------------

#define UPDATE_TWEAK4(tweak, I){ \
    tweak[I][0] = PERMUTE(tweak[I-1][0]);\
    tweak[I][1] = PERMUTE(tweak[I-1][1]);\
    tweak[I][2] = PERMUTE(tweak[I-1][2]);\
    tweak[I][3] = PERMUTE(tweak[I-1][3]);\
}

// ---------------------------------------------------------

#define ONE_XOR4( s, subkey, tweak ){\
    s[0] = XOR( s[0] , XOR(subkey , tweak[0][0]) );\
    s[1] = XOR( s[1] , XOR(subkey , tweak[0][1]) );\
    s[2] = XOR( s[2] , XOR(subkey , tweak[0][2]) );\
    s[3] = XOR( s[3] , XOR(subkey , tweak[0][3]) ); \
}

// ---------------------------------------------------------

#define ONE_XOR_FOO4( s, subkey, tweak ){\
    s[0] = XOR( s[0] , XOR(subkey , tweak[0]) );\
    s[1] = XOR( s[1] , XOR(subkey , tweak[1]) );\
    s[2] = XOR( s[2] , XOR(subkey , tweak[2]) );\
    s[3] = XOR( s[3] , XOR(subkey , tweak[3]) ); \
}

// ---------------------------------------------------------

#define ONE_XOR4_TWEAK_WORD( s, subkey, tweak ){\
    s[0] = XOR( s[0] , XOR(subkey , tweak) );\
    s[1] = XOR( s[1] , XOR(subkey , tweak) );\
    s[2] = XOR( s[2] , XOR(subkey , tweak) );\
    s[3] = XOR( s[3] , XOR(subkey , tweak) ); \
}

// ---------------------------------------------------------

#define ONE_XOR4_MOVE(d, s, s_i, subkey, tweak ){\
    d[0] = XOR( s[s_i] , XOR(subkey , tweak[0][0]) );\
    d[1] = XOR( s[s_i+1] , XOR(subkey , tweak[0][1]) );\
    d[2] = XOR( s[s_i+2] , XOR(subkey , tweak[0][2]) );\
    d[3] = XOR( s[s_i+3] , XOR(subkey , tweak[0][3]) ); \
}

// ---------------------------------------------------------

#define ONE_ROUND4( s, subkey, tweak , Round ){\
    s[0] = ENC( s[0] , XOR(subkey, tweak[Round][0] ) );\
    s[1] = ENC( s[1] , XOR(subkey, tweak[Round][1] ) );\
    s[2] = ENC( s[2] , XOR(subkey, tweak[Round][2] ) );\
    s[3] = ENC( s[3] , XOR(subkey, tweak[Round][3] ) );\
}

// ---------------------------------------------------------

#define ONE_ROUND4_TWEAK_WORD( s, subkey, tweak ){\
    s[0] = ENC( s[0] , XOR(subkey, tweak) );\
    s[1] = ENC( s[1] , XOR(subkey, tweak) );\
    s[2] = ENC( s[2] , XOR(subkey, tweak) );\
    s[3] = ENC( s[3] , XOR(subkey, tweak) );\
}

// ---------------------------------------------------------

#define LOAD_CTR8(ctr, tweaks, input) { \
    tweaks[0] = XOR(ctr, input); \
    tweaks[4] = vadd(tweaks[0], four_be); \
    tweaks[1] = vadd(tweaks[0], one_be); \
    tweaks[2] = vadd(tweaks[0], two_be); \
    tweaks[3] = vadd(tweaks[0], three_be); \
    tweaks[5] = vadd(tweaks[4], one_be); \
    tweaks[6] = vadd(tweaks[5], two_be); \
    tweaks[7] = vadd(tweaks[6], three_be); \
}

// ---------------------------------------------------------

#define LOAD_CTR1(ctr, tweak, input) { \
    tweak = vxor(ctr, input); \
}

// ---------------------------------------------------------

#define ADD8(tweaks, summand) { \
    tweaks[0] = vadd(tweaks[0], summand); \
    tweaks[1] = vadd(tweaks[1], summand); \
    tweaks[2] = vadd(tweaks[2], summand); \
    tweaks[3] = vadd(tweaks[3], summand); \
    tweaks[4] = vadd(tweaks[4], summand); \
    tweaks[5] = vadd(tweaks[5], summand); \
    tweaks[6] = vadd(tweaks[6], summand); \
    tweaks[7] = vadd(tweaks[7], summand); \
}

// ---------------------------------------------------------

#define LOAD_CTR4(ctr, tweaks, input) { \
    tweaks[0] = vxor(ctr, input); \
    tweaks[1] = vxor(tweaks[0], one_be); \
    tweaks[2] = vxor(tweaks[0], two_be); \
    tweaks[3] = vxor(tweaks[0], three_be); \
}

// ---------------------------------------------------------

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

// ---------------------------------------------------------

#define DEOXYS8( states, subkeys, tweak ) {\
  ONE_XOR8  ( states , subkeys[ 0] , tweak      );\
  ONE_ROUND8( states , subkeys[ 1] , tweak ,  1 );\
  ONE_ROUND8( states , subkeys[ 2] , tweak ,  2 );\
  ONE_ROUND8( states , subkeys[ 3] , tweak ,  3 );\
  ONE_ROUND8( states , subkeys[ 4] , tweak ,  4 );\
  ONE_ROUND8( states , subkeys[ 5] , tweak ,  5 );\
  ONE_ROUND8( states , subkeys[ 6] , tweak ,  6 );\
  ONE_ROUND8( states , subkeys[ 7] , tweak ,  7 );\
  ONE_ROUND8( states , subkeys[ 8] , tweak ,  0 );\
  ONE_ROUND8( states , subkeys[ 9] , tweak ,  1 );\
  ONE_ROUND8( states , subkeys[10] , tweak ,  2 );\
  ONE_ROUND8( states , subkeys[11] , tweak ,  3 );\
  ONE_ROUND8( states , subkeys[12] , tweak ,  4 );\
  ONE_ROUND8( states , subkeys[13] , tweak ,  5 );\
  ONE_ROUND8( states , subkeys[14] , tweak ,  6 );\
}

// ---------------------------------------------------------

#define ONE_ROUND8_FIXED_TWEAKEY( s, tweakey ){\
    s[0] = ENC( s[0] ,  tweakey );\
    s[1] = ENC( s[1] ,  tweakey );\
    s[2] = ENC( s[2] ,  tweakey );\
    s[3] = ENC( s[3] ,  tweakey );\
    s[4] = ENC( s[4] ,  tweakey );\
    s[5] = ENC( s[5] ,  tweakey );\
    s[6] = ENC( s[6] ,  tweakey );\
    s[7] = ENC( s[7] ,  tweakey );\
}
#define ONE_ROUND4_FIXED_TWEAKEY( s, tweakey ){\
    s[0] = ENC( s[0] ,  tweakey );\
    s[1] = ENC( s[1] ,  tweakey );\
    s[2] = ENC( s[2] ,  tweakey );\
    s[3] = ENC( s[3] ,  tweakey );\
}
// ---------------------------------------------------------
#define DEOXYS8_FIXED_TWEAKEY( states, roundtweakeys) {\
  ONE_ROUND8_FIXED_TWEAKEY( states , roundtweakeys[ 0] );\
  ONE_ROUND8_FIXED_TWEAKEY( states , roundtweakeys[ 1] );\
  ONE_ROUND8_FIXED_TWEAKEY( states , roundtweakeys[ 2] );\
  ONE_ROUND8_FIXED_TWEAKEY( states , roundtweakeys[ 3] );\
  ONE_ROUND8_FIXED_TWEAKEY( states , roundtweakeys[ 4] );\
  ONE_ROUND8_FIXED_TWEAKEY( states , roundtweakeys[ 5] );\
  ONE_ROUND8_FIXED_TWEAKEY( states , roundtweakeys[ 6] );\
  ONE_ROUND8_FIXED_TWEAKEY( states , roundtweakeys[ 7] );\
  ONE_ROUND8_FIXED_TWEAKEY( states , roundtweakeys[ 8] );\
  ONE_ROUND8_FIXED_TWEAKEY( states , roundtweakeys[ 9] );\
  ONE_ROUND8_FIXED_TWEAKEY( states , roundtweakeys[10] );\
  ONE_ROUND8_FIXED_TWEAKEY( states , roundtweakeys[11] );\
  ONE_ROUND8_FIXED_TWEAKEY( states , roundtweakeys[12] );\
  ONE_ROUND8_FIXED_TWEAKEY( states , roundtweakeys[13] );\
}
#define DEOXYS4_FIXED_TWEAKEY( states, roundtweakeys) {\
  ONE_ROUND4_FIXED_TWEAKEY( states , roundtweakeys[ 0] );\
  ONE_ROUND4_FIXED_TWEAKEY( states , roundtweakeys[ 1] );\
  ONE_ROUND4_FIXED_TWEAKEY( states , roundtweakeys[ 2] );\
  ONE_ROUND4_FIXED_TWEAKEY( states , roundtweakeys[ 3] );\
  ONE_ROUND4_FIXED_TWEAKEY( states , roundtweakeys[ 4] );\
  ONE_ROUND4_FIXED_TWEAKEY( states , roundtweakeys[ 5] );\
  ONE_ROUND4_FIXED_TWEAKEY( states , roundtweakeys[ 6] );\
  ONE_ROUND4_FIXED_TWEAKEY( states , roundtweakeys[ 7] );\
  ONE_ROUND4_FIXED_TWEAKEY( states , roundtweakeys[ 8] );\
  ONE_ROUND4_FIXED_TWEAKEY( states , roundtweakeys[ 9] );\
  ONE_ROUND4_FIXED_TWEAKEY( states , roundtweakeys[10] );\
  ONE_ROUND4_FIXED_TWEAKEY( states , roundtweakeys[11] );\
  ONE_ROUND4_FIXED_TWEAKEY( states , roundtweakeys[12] );\
  ONE_ROUND4_FIXED_TWEAKEY( states , roundtweakeys[13] );\
}

// ---------------------------------------------------------








// ---------------------------------------------------------

#define ADD8_PERMUTES(tweaks) {\
    tweaks[0] = vadd(tweaks[0], eight_be); \
    tweaks[1] = vadd(tweaks[1], eight_be_1); \
    tweaks[2] = vadd(tweaks[2], eight_be_2); \
    tweaks[3] = vadd(tweaks[3], eight_be_3); \
    tweaks[4] = vadd(tweaks[4], eight_be_4); \
    tweaks[5] = vadd(tweaks[5], eight_be_5); \
    tweaks[6] = vadd(tweaks[6], eight_be_6); \
    tweaks[7] = vadd(tweaks[7], eight_be_7); \
}

// ---------------------------------------------------------

#define ONE_ROUND_TWEAK_DEPENDENT8(s, subkey, tweak, tmp, one, two, three, four, five, six, seven){\
    tmp = XOR(subkey, tweak); \
    s[0] = ENC(s[0], tmp);\
    s[1] = ENC(s[1], XOR(tmp, one));\
    s[2] = ENC(s[2], XOR(tmp, two));\
    s[3] = ENC(s[3], XOR(tmp, three));\
    s[4] = ENC(s[4], XOR(tmp, four));\
    s[5] = ENC(s[5], XOR(tmp, five));\
    s[6] = ENC(s[6], XOR(tmp, six));\
    s[7] = ENC(s[7], XOR(tmp, seven));\
}

// ---------------------------------------------------------

#define ONE_XOR8_FIXED_INPUT(s, tweak, input){\
    s[0] = XOR(input, tweak); \
    s[1] = XOR(s[0], one_be); \
    s[2] = XOR(s[0], two_be); \
    s[3] = XOR(s[0], three_be); \
    s[4] = XOR(s[0], four_be); \
    s[5] = XOR(s[0], five_be); \
    s[6] = XOR(s[0], six_be); \
    s[7] = XOR(s[0], seven_be); \
}

// ---------------------------------------------------------

#define DEOXYS8_FIXED_INPUT(states, subkeys, tweaks, input, tmp) { \
  ONE_XOR8_FIXED_INPUT(states, tweaks[0], input); \
  ONE_ROUND_TWEAK_DEPENDENT8(states, subkeys[ 1], tweaks[1], tmp, one_be_1, two_be_1, three_be_1, four_be_1, five_be_1, six_be_1, seven_be_1); \
  ONE_ROUND_TWEAK_DEPENDENT8(states, subkeys[ 2], tweaks[2], tmp, one_be_2, two_be_2, three_be_2, four_be_2, five_be_2, six_be_2, seven_be_2); \
  ONE_ROUND_TWEAK_DEPENDENT8(states, subkeys[ 3], tweaks[3], tmp, one_be_3, two_be_3, three_be_3, four_be_3, five_be_3, six_be_3, seven_be_3); \
  ONE_ROUND_TWEAK_DEPENDENT8(states, subkeys[ 4], tweaks[4], tmp, one_be_4, two_be_4, three_be_4, four_be_4, five_be_4, six_be_4, seven_be_4); \
  ONE_ROUND_TWEAK_DEPENDENT8(states, subkeys[ 5], tweaks[5], tmp, one_be_5, two_be_5, three_be_5, four_be_5, five_be_5, six_be_5, seven_be_5); \
  ONE_ROUND_TWEAK_DEPENDENT8(states, subkeys[ 6], tweaks[6], tmp, one_be_6, two_be_6, three_be_6, four_be_6, five_be_6, six_be_6, seven_be_6); \
  ONE_ROUND_TWEAK_DEPENDENT8(states, subkeys[ 7], tweaks[7], tmp, one_be_7, two_be_7, three_be_7, four_be_7, five_be_7, six_be_7, seven_be_7); \
  ONE_ROUND_TWEAK_DEPENDENT8(states, subkeys[ 8], tweaks[0], tmp, one_be,   two_be,   three_be,   four_be,   five_be,   six_be,   seven_be); \
  ONE_ROUND_TWEAK_DEPENDENT8(states, subkeys[ 9], tweaks[1], tmp, one_be_1, two_be_1, three_be_1, four_be_1, five_be_1, six_be_1, seven_be_1); \
  ONE_ROUND_TWEAK_DEPENDENT8(states, subkeys[10], tweaks[2], tmp, one_be_2, two_be_2, three_be_2, four_be_2, five_be_2, six_be_2, seven_be_2); \
  ONE_ROUND_TWEAK_DEPENDENT8(states, subkeys[11], tweaks[3], tmp, one_be_3, two_be_3, three_be_3, four_be_3, five_be_3, six_be_3, seven_be_3); \
  ONE_ROUND_TWEAK_DEPENDENT8(states, subkeys[12], tweaks[4], tmp, one_be_4, two_be_4, three_be_4, four_be_4, five_be_4, six_be_4, seven_be_4); \
  ONE_ROUND_TWEAK_DEPENDENT8(states, subkeys[13], tweaks[5], tmp, one_be_5, two_be_5, three_be_5, four_be_5, five_be_5, six_be_5, seven_be_5); \
  ONE_ROUND_TWEAK_DEPENDENT8(states, subkeys[14], tweaks[6], tmp, one_be_6, two_be_6, three_be_6, four_be_6, five_be_6, six_be_6, seven_be_6); \
}

// ---------------------------------------------------------

#define LOAD8(states, inputs, index) {\
    states[0] = inputs[index]; \
    states[1] = inputs[index + 1]; \
    states[2] = inputs[index + 2]; \
    states[3] = inputs[index + 3]; \
    states[4] = inputs[index + 4]; \
    states[5] = inputs[index + 5]; \
    states[6] = inputs[index + 6]; \
    states[7] = inputs[index + 7]; \
}

// ---------------------------------------------------------

#define ONE_XOR8_HASH_INPUT(s, tweak){\
    s[0] = XOR(s[0], tweak); \
    s[1] = XOR(s[1], XOR(tweak, one_be)); \
    s[2] = XOR(s[2], XOR(tweak, two_be)); \
    s[3] = XOR(s[3], XOR(tweak, three_be)); \
    s[4] = XOR(s[4], XOR(tweak, four_be)); \
    s[5] = XOR(s[5], XOR(tweak, five_be)); \
    s[6] = XOR(s[6], XOR(tweak, six_be)); \
    s[7] = XOR(s[7], XOR(tweak, seven_be)); \
}

// ---------------------------------------------------------

#define DEOXYS8_HASH_INPUT(states, subkeys, tweaks, tmp) { \
  ONE_XOR8_HASH_INPUT(states, tweaks[0]); \
  ONE_ROUND_TWEAK_DEPENDENT8(states, subkeys[ 1], tweaks[1], tmp, one_be_1, two_be_1, three_be_1, four_be_1, five_be_1, six_be_1, seven_be_1); \
  ONE_ROUND_TWEAK_DEPENDENT8(states, subkeys[ 2], tweaks[2], tmp, one_be_2, two_be_2, three_be_2, four_be_2, five_be_2, six_be_2, seven_be_2); \
  ONE_ROUND_TWEAK_DEPENDENT8(states, subkeys[ 3], tweaks[3], tmp, one_be_3, two_be_3, three_be_3, four_be_3, five_be_3, six_be_3, seven_be_3); \
  ONE_ROUND_TWEAK_DEPENDENT8(states, subkeys[ 4], tweaks[4], tmp, one_be_4, two_be_4, three_be_4, four_be_4, five_be_4, six_be_4, seven_be_4); \
  ONE_ROUND_TWEAK_DEPENDENT8(states, subkeys[ 5], tweaks[5], tmp, one_be_5, two_be_5, three_be_5, four_be_5, five_be_5, six_be_5, seven_be_5); \
  ONE_ROUND_TWEAK_DEPENDENT8(states, subkeys[ 6], tweaks[6], tmp, one_be_6, two_be_6, three_be_6, four_be_6, five_be_6, six_be_6, seven_be_6); \
  ONE_ROUND_TWEAK_DEPENDENT8(states, subkeys[ 7], tweaks[7], tmp, one_be_7, two_be_7, three_be_7, four_be_7, five_be_7, six_be_7, seven_be_7); \
  ONE_ROUND_TWEAK_DEPENDENT8(states, subkeys[ 8], tweaks[0], tmp, one_be,   two_be,   three_be,   four_be,   five_be,   six_be,   seven_be); \
  ONE_ROUND_TWEAK_DEPENDENT8(states, subkeys[ 9], tweaks[1], tmp, one_be_1, two_be_1, three_be_1, four_be_1, five_be_1, six_be_1, seven_be_1); \
  ONE_ROUND_TWEAK_DEPENDENT8(states, subkeys[10], tweaks[2], tmp, one_be_2, two_be_2, three_be_2, four_be_2, five_be_2, six_be_2, seven_be_2); \
  ONE_ROUND_TWEAK_DEPENDENT8(states, subkeys[11], tweaks[3], tmp, one_be_3, two_be_3, three_be_3, four_be_3, five_be_3, six_be_3, seven_be_3); \
  ONE_ROUND_TWEAK_DEPENDENT8(states, subkeys[12], tweaks[4], tmp, one_be_4, two_be_4, three_be_4, four_be_4, five_be_4, six_be_4, seven_be_4); \
  ONE_ROUND_TWEAK_DEPENDENT8(states, subkeys[13], tweaks[5], tmp, one_be_5, two_be_5, three_be_5, four_be_5, five_be_5, six_be_5, seven_be_5); \
  ONE_ROUND_TWEAK_DEPENDENT8(states, subkeys[14], tweaks[6], tmp, one_be_6, two_be_6, three_be_6, four_be_6, five_be_6, six_be_6, seven_be_6); \
}

// ---------------------------------------------------------

#define ONE_ROUND_TWEAK_DEPENDENT4(s, subkey, tweak, tmp, one, two, three){\
    tmp = XOR(subkey, tweak); \
    s[0] = ENC(s[0], tmp);\
    s[1] = ENC(s[1], XOR(tmp, one));\
    s[2] = ENC(s[2], XOR(tmp, two));\
    s[3] = ENC(s[3], XOR(tmp, three));\
}

// ---------------------------------------------------------

#define ONE_XOR4_FIXED_INPUT(s, tweak, input){\
    s[0] = XOR(input, tweak); \
    s[1] = XOR(s[0], one_be); \
    s[2] = XOR(s[0], two_be); \
    s[3] = XOR(s[0], three_be); \
}

// ---------------------------------------------------------

#define DEOXYS4_FIXED_INPUT(states, subkeys, tweaks, input, tmp) { \
  ONE_XOR4_FIXED_INPUT(states, tweaks[0], input); \
  ONE_ROUND_TWEAK_DEPENDENT4(states, subkeys[ 1], tweaks[1], tmp, one_be_1, two_be_1, three_be_1); \
  ONE_ROUND_TWEAK_DEPENDENT4(states, subkeys[ 2], tweaks[2], tmp, one_be_2, two_be_2, three_be_2); \
  ONE_ROUND_TWEAK_DEPENDENT4(states, subkeys[ 3], tweaks[3], tmp, one_be_3, two_be_3, three_be_3); \
  ONE_ROUND_TWEAK_DEPENDENT4(states, subkeys[ 4], tweaks[4], tmp, one_be_4, two_be_4, three_be_4); \
  ONE_ROUND_TWEAK_DEPENDENT4(states, subkeys[ 5], tweaks[5], tmp, one_be_5, two_be_5, three_be_5); \
  ONE_ROUND_TWEAK_DEPENDENT4(states, subkeys[ 6], tweaks[6], tmp, one_be_6, two_be_6, three_be_6); \
  ONE_ROUND_TWEAK_DEPENDENT4(states, subkeys[ 7], tweaks[7], tmp, one_be_7, two_be_7, three_be_7); \
  ONE_ROUND_TWEAK_DEPENDENT4(states, subkeys[ 8], tweaks[0], tmp, one_be,   two_be,   three_be); \
  ONE_ROUND_TWEAK_DEPENDENT4(states, subkeys[ 9], tweaks[1], tmp, one_be_1, two_be_1, three_be_1); \
  ONE_ROUND_TWEAK_DEPENDENT4(states, subkeys[10], tweaks[2], tmp, one_be_2, two_be_2, three_be_2); \
  ONE_ROUND_TWEAK_DEPENDENT4(states, subkeys[11], tweaks[3], tmp, one_be_3, two_be_3, three_be_3); \
  ONE_ROUND_TWEAK_DEPENDENT4(states, subkeys[12], tweaks[4], tmp, one_be_4, two_be_4, three_be_4); \
  ONE_ROUND_TWEAK_DEPENDENT4(states, subkeys[13], tweaks[5], tmp, one_be_5, two_be_5, three_be_5); \
  ONE_ROUND_TWEAK_DEPENDENT4(states, subkeys[14], tweaks[6], tmp, one_be_6, two_be_6, three_be_6); \
}

// ---------------------------------------------------------

#define LOAD4(states, inputs, index) {\
    states[0] = inputs[index]; \
    states[1] = inputs[index + 1]; \
    states[2] = inputs[index + 2]; \
    states[3] = inputs[index + 3]; \
}

// ---------------------------------------------------------

#define ONE_XOR4_HASH_INPUT(s, tweak){\
    s[0] = XOR(s[0], tweak); \
    s[1] = XOR(s[1], XOR(tweak, one_be)); \
    s[2] = XOR(s[2], XOR(tweak, two_be)); \
    s[3] = XOR(s[3], XOR(tweak, three_be)); \
}

// ---------------------------------------------------------

#define DEOXYS4_HASH_INPUT(states, subkeys, tweaks, tmp) { \
  ONE_XOR4_HASH_INPUT(states, tweaks[0]); \
  ONE_ROUND_TWEAK_DEPENDENT4(states, subkeys[ 1], tweaks[1], tmp, one_be_1, two_be_1, three_be_1); \
  ONE_ROUND_TWEAK_DEPENDENT4(states, subkeys[ 2], tweaks[2], tmp, one_be_2, two_be_2, three_be_2); \
  ONE_ROUND_TWEAK_DEPENDENT4(states, subkeys[ 3], tweaks[3], tmp, one_be_3, two_be_3, three_be_3); \
  ONE_ROUND_TWEAK_DEPENDENT4(states, subkeys[ 4], tweaks[4], tmp, one_be_4, two_be_4, three_be_4); \
  ONE_ROUND_TWEAK_DEPENDENT4(states, subkeys[ 5], tweaks[5], tmp, one_be_5, two_be_5, three_be_5); \
  ONE_ROUND_TWEAK_DEPENDENT4(states, subkeys[ 6], tweaks[6], tmp, one_be_6, two_be_6, three_be_6); \
  ONE_ROUND_TWEAK_DEPENDENT4(states, subkeys[ 7], tweaks[7], tmp, one_be_7, two_be_7, three_be_7); \
  ONE_ROUND_TWEAK_DEPENDENT4(states, subkeys[ 8], tweaks[0], tmp, one_be,   two_be,   three_be); \
  ONE_ROUND_TWEAK_DEPENDENT4(states, subkeys[ 9], tweaks[1], tmp, one_be_1, two_be_1, three_be_1); \
  ONE_ROUND_TWEAK_DEPENDENT4(states, subkeys[10], tweaks[2], tmp, one_be_2, two_be_2, three_be_2); \
  ONE_ROUND_TWEAK_DEPENDENT4(states, subkeys[11], tweaks[3], tmp, one_be_3, two_be_3, three_be_3); \
  ONE_ROUND_TWEAK_DEPENDENT4(states, subkeys[12], tweaks[4], tmp, one_be_4, two_be_4, three_be_4); \
  ONE_ROUND_TWEAK_DEPENDENT4(states, subkeys[13], tweaks[5], tmp, one_be_5, two_be_5, three_be_5); \
  ONE_ROUND_TWEAK_DEPENDENT4(states, subkeys[14], tweaks[6], tmp, one_be_6, two_be_6, three_be_6); \
}

// ---------------------------------------------------------

// Tweakable AES
#define TAES( s , subkeys , realtweak, t) { \
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
    s = ENC( s , XOR( subkeys[14] , t ) ); \
}
// ---------------------------------------------------------
#define TAES_FIXED_TWEAKEY( s , tweakey)\
    s = ENC( s , tweakey[ 0] );\
    s = ENC( s , tweakey[ 1] );\
    s = ENC( s , tweakey[ 2] );\
    s = ENC( s , tweakey[ 3] );\
    s = ENC( s , tweakey[ 4] );\
    s = ENC( s , tweakey[ 5] );\
    s = ENC( s , tweakey[ 6] );\
    s = ENC( s , tweakey[ 7] );\
    s = ENC( s , tweakey[ 8] );\
    s = ENC( s , tweakey[ 9] );\
    s = ENC( s , tweakey[10] );\
    s = ENC( s , tweakey[11] );\
    s = ENC( s , tweakey[12] );\
    s = ENC( s , tweakey[13] );


// Tweakable AES decryption
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
// Functions
// ---------------------------------------------------------

void DEOXYS_128_256_setup_key(const unsigned char *mkey, BLOCK *key);

// ---------------------------------------------------------

void DEOXYS_128_256_setup_tweak(const unsigned char *mkey, BLOCK *key);

// ---------------------------------------------------------

//Also works for decryption tweak setup
void DEOXYS_128_256_setup_key_decryption(BLOCK *dkey, BLOCK *ekey );

// ---------------------------------------------------------

void DEOXYS_128_256_encrypt(const BLOCK *rks, 
                            const BLOCK *rts,
                            const unsigned char *in, 
                            unsigned char *out);

// ---------------------------------------------------------

void DEOXYS_128_256_decrypt(const BLOCK *rks, 
                            const BLOCK *rts,
                            const unsigned char *in, 
                            unsigned char *out);

// ---------------------------------------------------------

#endif // _DEOXYS_BC_H
