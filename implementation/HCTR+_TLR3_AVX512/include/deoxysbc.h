#include "setup.h"

#define DEOXYS_BC_128_256_NUM_ROUNDS      14
#define DEOXYS_BC_128_256_NUM_ROUND_KEYS  (DEOXYS_BC_128_256_NUM_ROUNDS+1)

#define H_PERMUTATION _mm_setr_epi8( 1,6,11,12,5,10,15,0,9,14,3,4,13,2,7,8)
#define H_PERMUTATION_INV _mm_setr_epi8(7,0,13,10, 11,4,1,14, 15,8,5,2, 3,12,9,6)
#define H_PERMUTATION_14 _mm_setr_epi8(14,7,12,5,2,11,0,9,6,15,4,13,10,3,8,1)

#define H_PERMUTATION_512 _mm512_set_epi8(8, 7, 2, 13, 4, 3, 14, 9, 0, 15, 10, 5, 12, 11, 6, 1, 8, 7, 2, 13, 4, 3, 14, 9, 0, 15, 10, 5, 12, 11, 6, 1, 8, 7, 2, 13, 4, 3, 14, 9, 0, 15, 10, 5, 12, 11, 6, 1, 8, 7, 2, 13, 4, 3, 14, 9, 0, 15, 10, 5, 12, 11, 6, 1)

#define PERMUTE(x)     _mm_shuffle_epi8(x, H_PERMUTATION  )
#define PERMUTE_512(x) _mm512_shuffle_epi8(x, H_PERMUTATION_512  )
#define PERMUTEINV(x)  _mm_shuffle_epi8(x, H_PERMUTATION_INV)
#define PERMUTE14(x)   _mm_shuffle_epi8(x, H_PERMUTATION_14)

/*---------------From SUPERCOP DEOXYSBC---------------------------------------*/
#define ONE_XOR4( s, subkey, tweak )\
    s[0] = _mm512_xor_si512(s[0], _mm512_xor_si512(subkey, tweak[0][0]));\
    s[1] = _mm512_xor_si512(s[1], _mm512_xor_si512(subkey, tweak[0][1]));\
    s[2] = _mm512_xor_si512(s[2], _mm512_xor_si512(subkey, tweak[0][2]));\
    s[3] = _mm512_xor_si512(s[3], _mm512_xor_si512(subkey, tweak[0][3]));\
;
#define ONE_ROUND4( s, subkey, tweak , Round )\
    s[0] = _mm512_aesenc_epi128(s[0], _mm512_xor_si512(subkey, tweak[Round][0]));\
    s[1] = _mm512_aesenc_epi128(s[1], _mm512_xor_si512(subkey, tweak[Round][1]));\
    s[2] = _mm512_aesenc_epi128(s[2], _mm512_xor_si512(subkey, tweak[Round][2]));\
    s[3] = _mm512_aesenc_epi128(s[3], _mm512_xor_si512(subkey, tweak[Round][3]));\
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
#define ONE_XOR2( s, subkey, tweak )\
    s[0] = _mm512_xor_si512(s[0], _mm512_xor_si512(subkey, tweak[0][0]));\
    s[1] = _mm512_xor_si512(s[1], _mm512_xor_si512(subkey, tweak[0][1]));\
;
#define ONE_ROUND2( s, subkey, tweak , Round )\
    s[0] = _mm512_aesenc_epi128(s[0], _mm512_xor_si512(subkey, tweak[Round][0]));\
    s[1] = _mm512_aesenc_epi128(s[1], _mm512_xor_si512(subkey, tweak[Round][1]));\
;
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

void DEOXYS_128_256_setup_key(const unsigned char *mkey, BLOCK *key);
void DEOXYS_128_256_setup_tweak(const unsigned char *mkey, BLOCK *key);
//Also works for decryption tweak setup
void DEOXYS_128_256_setup_key_decryption(BLOCK *dkey, BLOCK *ekey );
void DEOXYS_128_256_encrypt(const BLOCK *rks, const BLOCK *rts,
                   const unsigned char *in, unsigned char *out);
void DEOXYS_128_256_decrypt(const BLOCK *rks, const BLOCK *rts,
                   const unsigned char *in, unsigned char *out);


