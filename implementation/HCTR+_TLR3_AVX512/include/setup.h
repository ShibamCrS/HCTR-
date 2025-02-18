#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include <emmintrin.h>
#include <immintrin.h>
#include <smmintrin.h>

// ---------------------------------------------------------------------
// Notions from other implementation
// ---------------------------------------------------------------------

#define CIPHERINFO "%s_AES_NI_%s" //ex: ctr_AES_NI__GNU_C_12.2.0_x86_64 
#define ENC(a,b)       _mm_aesenc_si128(a,b)
#define DEC(a,b)       _mm_aesdec_si128(a,b)
#define DECLAST(a,b)   _mm_aesdeclast_si128(a,b)
#define MCINV(a)       _mm_aesimc_si128(a)
#define XOR(a,b)       _mm_xor_si128(a,b)
#define AND(a,b)       _mm_and_si128(a,b)
#define OR(a,b)        _mm_or_si128(a,b)
#define ADD_ONE(b)     _mm_add_epi64(b,_mm_set_epi64x(0UL,1UL))
#define ADD_EIGHT(b)   _mm_add_epi64(b,_mm_set_epi64x(0UL,8UL))
#define ZERO()         _mm_setzero_si128()
#define ONE     _mm_set_epi8( 0b00100000, 0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 )
#define TWO     _mm_set_epi8( 0b01000000, 0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 )
#define THREE   _mm_set_epi8( 0b01100000, 0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 )
#define FOUR    _mm_set_epi8( 0b10000000, 0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 )

#define MASKD   _mm_set_epi8( 0x1F, 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF )
#define TRUNC(a, b) XOR(AND(a, MASKD), b)

#if __GNUC__
    #define GCC_VERSION (__GNUC__ * 10 + __GNUC_MINOR__)
    #define ALIGN(n) __attribute__ ((aligned(n)))
    #define inline __inline__
    #define restrict __restrict__
#else /* Not GNU/Microsoft/C99: delete alignment/inline/restrict uses.     */
    #define ALIGN(n)
    #define inline
    #define restrict
#endif

/* typedef __m128i BLOCK; */
typedef ALIGN(16) __m128i BLOCK;
// ---------------------------------------------------------------------


#define load(x)                _mm_load_si128((__m128i*)(x))
#define loadu(x)               _mm_loadu_si128((__m128i*)(x))
#define store(dst, x)          _mm_store_si128((__m128i*)(dst), x)
#define storeu(dst, x)         _mm_storeu_si128((__m128i*)(dst), x)

#define set8(x15, x14, x13, x12, x11, x10, x9, x8, x7, x6, x5, x4, x3, x2, x1, x0) \
    _mm_set_epi8((char)x15, (char)x14, (char)x13, (char)x12, (char)x11, (char)x10, (char)x9, (char)x8, (char)x7, (char)x6, (char)x5, (char)x4, (char)x3, (char)x2, (char)x1, (char)x0)
#define set8r(x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12, x13, x14, x15) \
    _mm_setr_epi8((char)x0, (char)x1, (char)x2, (char)x3, (char)x4, (char)x5, (char)x6, (char)x7, (char)x8, (char)x9, (char)x10, (char)x11, (char)x12, (char)x13, (char)x14, (char)x15)
#define set32(x3, x2, x1, x0)  _mm_set_epi32(x3, x2, x1, x0)
#define setr32(x0, x1, x2, x3) _mm_setr_epi32(x0, x1, x2, x3)
#define set64(x1, x0)          _mm_set_epi64(x1, x0)
#define set1(x)                _mm_set1_epi8((char)x)

#define zero                   _mm_setzero_si128()
#define all_ones               set1(0xFF)
#define one_be                 set32(0, 0, 0, 1)
#define two_be                 set32(0, 0, 0, 2)
#define three_be               set32(0, 0, 0, 3)
#define four_be                set32(0, 0, 0, 4)
#define five_be                set32(0, 0, 0, 5)
#define six_be                 set32(0, 0, 0, 6)
#define seven_be               set32(0, 0, 0, 7)
#define eight_be               set32(0, 0, 0, 8)

#define one_le                 setr32(0, 0, 0, 1)
#define two_le                 setr32(0, 0, 0, 2)
#define three_le               setr32(0, 0, 0, 3)
#define four_le                setr32(0, 0, 0, 4)
#define five_le                setr32(0, 0, 0, 5)
#define six_le                 setr32(0, 0, 0, 6)
#define seven_le               setr32(0, 0, 0, 7)
#define eight_le               setr32(0, 0, 0, 8)
#define sixteen_le             setr32(0, 0, 0, 16)
#define next_byte_le           setr32(0, 0, 0, 0x100)
#define next_byte_two_le       setr32(0, 0, 0, 0x200)
#define next_byte_three_le     setr32(0, 0, 0, 0x300)
#define next_byte_four_le      setr32(0, 0, 0, 0x400)
#define next_byte_five_le      setr32(0, 0, 0, 0x500)
#define next_byte_eight_le     setr32(0, 0, 0, 0x800)
#define next_byte_fifteen_le   setr32(0, 0, 0, 0xf00)

#define vxor(x, y)             _mm_xor_si128(x, y)
#define vadd(x,y)              _mm_add_epi64(x, y)
#define vadd32(x,y)            _mm_add_epi32(x, y)
#define vand(x,y)              _mm_and_si128(x, y)
#define vandnot(x,y)           _mm_andnot_si128(x, y)
#define vor(x,y)               _mm_or_si128(x, y)
#define vxor3(x,y,z)           vxor(x, vxor(y, z))
#define vxor4(w,x,y,z)         vxor(vxor(x, y), vxor(w, z))
#define vflip_all(x)           vxor(x, all_ones)

#define vextract64(x, i)       _mm_extract_epi64(x, i)

#define vshiftl16(x, r)        _mm_slli_epi16(x, r)
#define vshiftl32(x, r)        _mm_slli_epi32(x, r)
#define vshiftl64(x, r)        _mm_slli_epi64(x, r)
#define vshiftr16(x, r)        _mm_srli_epi16(x, r)
#define vshiftr32(x, r)        _mm_srli_epi32(x, r)
#define vshiftr64(x, r)        _mm_srli_epi64(x, r)

#define vshiftl_bytes(x, r)    _mm_bslli_si128(x, r)
#define vshiftr_bytes(x, r)    _mm_bsrli_si128(x, r)

#define vshiftr16_sign(x, r)   _mm_srai_epi16(x, r)
#define vshiftr64_sign(x, r)   _mm_srai_epi64(x, r)

#define vpermute_words32(x, mask)  _mm_permute_ps(x, mask)
#define vunpack_lo64(x0, x1)   _mm_unpacklo_epi64(x0, x1)
#define vunpack_hi64(x0, x1)   _mm_unpackhi_epi64(x0, x1)

#define aesdec(x, k)           _mm_aesdec_si128(x, k)
#define aesdeclast(x, k)       _mm_aesdeclast_si128(x, k)
#define aesenc(x, k)           _mm_aesenc_si128(x, k)
#define aesenclast(x, k)       _mm_aesenclast_si128(x, k)
#define aesimc(x)              _mm_aesimc_si128(x)
#define aeskeygenassist(x, rcon) _mm_aeskeygenassist_si128(x, rcon)
#define clmul(x, y, mask)      _mm_clmulepi64_si128(x, y, mask)

#define permute(x, p)          _mm_shuffle_epi8(x, p)

#define increment(x)           vadd(x, one)
#define increment_le(x)        vadd32(x, one_le)
#define increment2_le(x)       vadd32(x, two_le)
#define increment3_le(x)       vadd32(x, three_le)
#define increment4_le(x)       vadd32(x, four_le)
#define increment5_le(x)       vadd32(x, five_le)
#define increment8_le(x)       vadd32(x, eight_le)
#define increment16_le(x)      vadd32(x, sixteen_le)
#define increment_next_byte_le(x)    vadd32(x, next_byte_le)
#define increment2_next_byte_le(x)   vadd32(x, next_byte_two_le)
#define increment4_next_byte_le(x)   vadd32(x, next_byte_four_le)
#define increment5_next_byte_le(x)   vadd32(x, next_byte_five_le)
#define increment8_next_byte_le(x)   vadd32(x, next_byte_eight_le)
#define increment15_next_byte_le(x)  vadd32(x, next_byte_fifteen_le)

// Returns y that is a 32-bit-selected version of x.
// mask is 8-bit value x7 x6 ... x1 x0, where each subsequent 2-bit
// represents a value i in {0, 1, 2, 3} that says that at the i-th
// 32-bit word shall be chosen for 32-bit words y_i with 
// y = y_3 y_2 y_1 y_0 (32-bit words, y_0 lowest)
// y_0 = x_{x1 || x0}
// y_1 = x_{x3 || x2}
// y_2 = x_{x5 || x4}
// y_3 = x_{x7 || x6}
#define shuffle32(x, mask)   _mm_shuffle_epi32(x, mask)
// 0x4e = 01 00 11 10 = swap 64-bit halves
#define swap64(x)            _mm_shuffle_epi32(x, 0x4e)

// ---------------------------------------------------------------------

#define xor_eight(dst_array, src_array, constant) do { \
    dst[0] = vxor(constant, src_array[0]); \
    dst[1] = vxor(constant, src_array[1]); \
    dst[2] = vxor(constant, src_array[2]); \
    dst[3] = vxor(constant, src_array[3]); \
    dst[4] = vxor(constant, src_array[4]); \
    dst[5] = vxor(constant, src_array[5]); \
    dst[6] = vxor(constant, src_array[6]); \
    dst[7] = vxor(constant, src_array[7]); \
} while (0)

// ---------------------------------------------------------------------

#define load_xor_and_store_eight(dst_array, to_xor_array, to_load_array) do { \
    dst_array[0] = vxor(to_xor_array[0], loadu(to_load_array  )); \
    dst_array[1] = vxor(to_xor_array[1], loadu(to_load_array+1)); \
    dst_array[2] = vxor(to_xor_array[2], loadu(to_load_array+2)); \
    dst_array[3] = vxor(to_xor_array[3], loadu(to_load_array+3)); \
    dst_array[4] = vxor(to_xor_array[4], loadu(to_load_array+4)); \
    dst_array[5] = vxor(to_xor_array[5], loadu(to_load_array+5)); \
    dst_array[6] = vxor(to_xor_array[6], loadu(to_load_array+6)); \
    dst_array[7] = vxor(to_xor_array[7], loadu(to_load_array+7)); \
} while (0)

// ---------------------------------------------------------------------

#define aesenc_eight(dst_array, t, k, tmp) do { \
    tmp = vxor(t, k);                                         \
    dst_array[0] = aesenc(dst_array[0], tmp);   \
    dst_array[1] = aesenc(dst_array[1], tmp);   \
    dst_array[2] = aesenc(dst_array[2], tmp);   \
    dst_array[3] = aesenc(dst_array[3], tmp);   \
    dst_array[4] = aesenc(dst_array[4], tmp);   \
    dst_array[5] = aesenc(dst_array[5], tmp);   \
    dst_array[6] = aesenc(dst_array[6], tmp);   \
    dst_array[7] = aesenc(dst_array[7], tmp);   \
} while (0)

/*
    computing:
    y_new = y_old + x[0] + x[1] + x[2] + x[3]
*/
#define accumulate_four_stateful(y, x) { \
    x[0] = XOR(x[0], x[1]); \
    x[2] = XOR(x[2], x[3]); \
    x[3] = XOR(x[0], x[2]); \
    y    = XOR(y, x[3]); \
}
#define accumulate_eight_stateful(y, x) { \
    x[0] = XOR(x[0], x[1]); \
    x[2] = XOR(x[2], x[3]); \
    x[4] = XOR(x[4], x[5]); \
    x[6] = XOR(x[6], x[7]); \
    x[0] = XOR(x[0], x[2]); \
    x[4] = XOR(x[4], x[6]); \
    x[3] = XOR(x[0], x[4]); \
    y    = XOR(y, x[3]); \
}

#define REDUCTION_POLYNOMIAL  _mm_set_epi32(0, 0, 0, 135)
#define accumulate_four(x, y) { \
    x[0] = XOR(x[0], x[1]); \
    x[2] = XOR(x[2], x[3]); \
    y    = XOR(x[0], x[2]); \
}
#define accumulate_eight(x, y) { \
    x[0] = XOR(x[0], x[1]); \
    x[2] = XOR(x[2], x[3]); \
    x[4] = XOR(x[4], x[5]); \
    x[6] = XOR(x[6], x[7]); \
    x[0] = XOR(x[0], x[2]); \
    x[4] = XOR(x[4], x[6]); \
    y    = XOR(x[0], x[4]); \
}
#define accumulate_16(x, y) { \
    x[0]  = XOR(x[0], x[1]); \
    x[2]  = XOR(x[2], x[3]); \
    x[4]  = XOR(x[4], x[5]); \
    x[6]  = XOR(x[6], x[7]); \
    x[8]  = XOR(x[8], x[9]); \
    x[10] = XOR(x[10], x[11]); \
    x[12] = XOR(x[12], x[13]); \
    x[14] = XOR(x[14], x[15]); \
    x[0]  = XOR(x[0], x[2]); \
    x[4]  = XOR(x[4], x[6]); \
    x[8]  = XOR(x[8], x[10]); \
    x[12] = XOR(x[12], x[14]); \
    x[0]  = XOR(x[0], x[4]); \
    x[8]  = XOR(x[8], x[12]); \
       y  = XOR(x[0], x[8]); \
}

#define accumulate_16_stateful(y, x) { \
    x[0]  = XOR(x[0], x[1]); \
    x[2]  = XOR(x[2], x[3]); \
    x[4]  = XOR(x[4], x[5]); \
    x[6]  = XOR(x[6], x[7]); \
    x[8]  = XOR(x[8], x[9]); \
    x[10] = XOR(x[10], x[11]); \
    x[12] = XOR(x[12], x[13]); \
    x[14] = XOR(x[14], x[15]); \
    x[0]  = XOR(x[0], x[2]); \
    x[4]  = XOR(x[4], x[6]); \
    x[8]  = XOR(x[8], x[10]); \
    x[12] = XOR(x[12], x[14]); \
    x[0]  = XOR(x[0], x[4]); \
    x[8]  = XOR(x[8], x[12]); \
    x[0]  = XOR(x[0], x[8]); \
    y     = XOR(y, x[0]); \
}
#define XOR4_four(X, Y) do { \
    X[0] = XOR4(X[0], Y[0]); \
    X[1] = XOR4(X[1], Y[1]); \
    X[2] = XOR4(X[2], Y[2]); \
    X[3] = XOR4(X[3], Y[3]); \
} while (0)
#define XOR4_two(X, Y) do { \
    X[0] = XOR4(X[0], Y[0]); \
    X[1] = XOR4(X[1], Y[1]); \
} while (0)

#define TRUNC4_four(X, Y, Z, W) do { \
    X = AND4(X, MASKD4); \
    Y = AND4(Y, MASKD4); \
    Z = AND4(Z, MASKD4); \
    W = AND4(W, MASKD4); \
    X = XOR4(X, ONE4); \
    Y = XOR4(Y, ONE4); \
    Z = XOR4(Z, ONE4); \
    W = XOR4(W, ONE4); \
} while (0)
#define TRUNC4_two(X, Y) do { \
    X = AND4(X, MASKD4); \
    Y = AND4(Y, MASKD4); \
    X = XOR4(X, ONE4); \
    Y = XOR4(Y, ONE4); \
} while (0)

// ---------------------------------------------------------
