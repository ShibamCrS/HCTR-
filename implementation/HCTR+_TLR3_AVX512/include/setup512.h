#include <stdint.h>
#include <tmmintrin.h>        /* SSSE3 instructions              */
#include <xmmintrin.h>        /* SSE instructions and _mm_malloc */
#include <emmintrin.h>        /* SSE2 instructions               */
#include <wmmintrin.h>
#include <immintrin.h>

#include "setup.h"

#if 0 //defined in setup.h
#define CIPHERINFO "%s_AES_NI_%s" //ex: ctr_AES_NI__GNU_C_12.2.0_x86_64 
#define MCINV(a)       _mm_aesimc_si128(a)
#define ENC(a,b)       _mm_aesenc_si128(a,b)
#define DEC(a,b)       _mm_aesdec_si128(a,b)
#define DECLAST(a,b)   _mm_aesdeclast_si128(a,b)
#define XOR(a,b)       _mm_xor_si128(a,b)
#define AND(a,b)       _mm_and_si128(a,b)
#define OR(a,b)        _mm_or_si128(a,b)
#define ADD_ONE(b)     _mm_add_epi32(b,_mm_set_epi32(0,0,0,1))

#define ZERO()         _mm_setzero_si128()
#define ONE     _mm_set_epi8( 0b00100000, 0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 )
#define TWO     _mm_set_epi8( 0b01000000, 0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 )
#define THREE   _mm_set_epi8( 0b01100000, 0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 )
#define FOUR    _mm_set_epi8( 0b10000000, 0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 )
#define MASKD   _mm_set_epi8( 0b00011111, 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF )
#define TRUNC(a, b) XOR(AND(a, MASKD), b)
#endif

#define ENC4(a,b)      _mm512_aesenc_epi128(a,b)
#define DEC4(a,b)      _mm512_aesdec_epi128(a,b)
#define DECLAST4(a,b)  _mm512_aesdeclast_epi128(a,b)
#define XOR4(a,b)      _mm512_xor_si512(a,b)
#define AND4(a,b)      _mm512_and_si512(a,b)
#define ADD4(a,b)      _mm512_add_epi64(a,b)

#define ZERO4()        _mm512_setzero_si512()
#define CTR4444        _mm512_set_epi64(0,4, 0, 4, 0, 4, 0, 4)
#define CTR0123        _mm512_set_epi64(0,0, 0,-1, 0,-2, 0,-3)
#define CTR4321        _mm512_set_epi64(0,4, 0, 3, 0, 2, 0, 1)
#define CTR3210        _mm512_set_epi64(0,3, 0, 2, 0, 1, 0, 0)
#define ADD4444(b)     _mm512_add_epi64(b, CTR4444)
#define ADD3210(b)     _mm512_add_epi64(b, CTR3210)
#define ADD0123(b)     _mm512_add_epi64(b, CTR0123)
#define ADD4321(b)     _mm512_add_epi64(b, CTR4321)

#define ONE4    _mm512_set_epi8( 0b00100000, 0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0b00100000, 0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0b00100000, 0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0b00100000, 0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 )
#define MASKD4   _mm512_set_epi8( 0b00011111, 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF, 0b00011111, 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF, 0b00011111, 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF, 0b00011111, 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF )
#define TRUNC4(a, b) XOR4(AND4(a, MASKD4), b)

#define ONE4    _mm512_set_epi8( 0b00100000, 0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0b00100000, 0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0b00100000, 0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0b00100000, 0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 )
#define MASKD4   _mm512_set_epi8( 0b00011111, 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF, 0b00011111, 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF, 0b00011111, 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF, 0b00011111, 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF )
#define TRUNC4(a, b) XOR4(AND4(a, MASKD4), b)

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

typedef ALIGN(16) __m512i BLOCK4;
/* typedef __m512i BLOCK4; */

// ---------------------------------------------------------------------

#define zload(x)                _mm_load_si512((__m512i*)(x))
#define zloadu(x)               _mm_loadu_si512((__m512i*)(x))
#define zstore(dst, x)          _mm_store_si512((__m512i*)(dst), x)
#define zstoreu(dst, x)         _mm_storeu_si512((__m512i*)(dst), x)

#define zbroadcast(x)           _mm512_broadcast_i64x2(x)

// ---------------------------------------------------------------------

// ---------------------------------------------------------

#define LOAD_512_2(states, inputs, index) {\
    states[0] = inputs[index]; \
    states[1] = inputs[index + 1]; \
}

// ---------------------------------------------------------

#define LOAD_512(states, inputs, index) {\
    states[0] = inputs[index]; \
}

// ---------------------------------------------------------
#define xor512_eight(dst, src_array, constant) do { \
    dst[0] = XOR4(constant, src_array[0]); \
    dst[1] = XOR4(constant, src_array[1]); \
} while (0)
