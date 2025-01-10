#include <stdint.h>
#include <tmmintrin.h>        /* SSSE3 instructions              */
#include <xmmintrin.h>        /* SSE instructions and _mm_malloc */
#include <emmintrin.h>        /* SSE2 instructions               */
#include <wmmintrin.h>
#include <immintrin.h>

#define CIPHERINFO "%s_AES_NI_%s" //ex: ctr_AES_NI__GNU_C_12.2.0_x86_64 
#define ENC(a,b)       _mm_aesenc_si128(a,b)
#define DEC(a,b)       _mm_aesdec_si128(a,b)
#define DECLAST(a,b)   _mm_aesdeclast_si128(a,b)
#define MCINV(a)       _mm_aesimc_si128(a)
#define XOR(a,b)       _mm_xor_si128(a,b)
#define XOR4(a,b)      _mm512_xor_si512(a,b)
#define AND(a,b)       _mm_and_si128(a,b)
#define AND4(a,b)      _mm512_and_si512(a,b)
#define OR(a,b)        _mm_or_si128(a,b)
#define ZERO()         _mm_setzero_si128()
#define ADD_ONE(b)     _mm_add_epi32(b,_mm_set_epi32(0,0,0,1))

#if 0
#define ZERO4()        _mm512_setzero_si512()
#define CTR4444        _mm512_set_epi32(0,0,0,4, 0,0,0,4, 0,0,0,4, 0,0,0,4)
#define CTR0123        _mm512_set_epi32(0,0,0,0, 0,0,0,-1, 0,0,0,-2, 0,0,0,-3)
#define CTR4321        _mm512_set_epi32(0,0,0,4, 0,0,0,3, 0,0,0,2, 0,0,0,1)
#define CTR3210        _mm512_set_epi32(0,0,0,3, 0,0,0,2, 0,0,0,1, 0,0,0,0)
#define ADD4444(b)     _mm512_add_epi32(b, CTR4444)
#define ADD3210(b)     _mm512_add_epi32(b, CTR3210)
#define ADD0123(b)     _mm512_add_epi32(b, CTR0123)
#define ADD4321(b)     _mm512_add_epi32(b, CTR4321)
#endif

#if 1
#define ZERO4()        _mm512_setzero_si512()
#define CTR4444        _mm512_set_epi64(0,4, 0, 4, 0, 4, 0, 4)
#define CTR0123        _mm512_set_epi64(0,0, 0,-1, 0,-2, 0,-3)
#define CTR4321        _mm512_set_epi64(0,4, 0, 3, 0, 2, 0, 1)
#define CTR3210        _mm512_set_epi64(0,3, 0, 2, 0, 1, 0, 0)
#define ADD4444(b)     _mm512_add_epi64(b, CTR4444)
#define ADD3210(b)     _mm512_add_epi64(b, CTR3210)
#define ADD0123(b)     _mm512_add_epi64(b, CTR0123)
#define ADD4321(b)     _mm512_add_epi64(b, CTR4321)
#endif


#define ONE     _mm_set_epi8( 0b00100000, 0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 )
#define TWO     _mm_set_epi8( 0b01000000, 0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 )
#define THREE   _mm_set_epi8( 0b01100000, 0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 )
#define FOUR    _mm_set_epi8( 0b10000000, 0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 )
#define MASKD   _mm_set_epi8( 0b00011111, 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF )
#define TRUNC(a, b) XOR(AND(a, MASKD), b)


#define ONE4    _mm512_set_epi8( 0b00100000, 0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0b00100000, 0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0b00100000, 0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0b00100000, 0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 )
#define MASKD4   _mm512_set_epi8( 0b00011111, 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF, 0b00011111, 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF, 0b00011111, 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF, 0b00011111, 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF )
#define TRUNC4(a, b) XOR4(AND4(a, MASKD4), b)
#define swap_if_le(b) _mm_shuffle_epi8(b,_mm_set_epi8(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15))

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

typedef ALIGN(16) __m128i BLOCK;
typedef ALIGN(16) __m512i BLOCK4;
/* These determine how to allocate 16-byte aligned vectors, if needed.     */
#define USE_MM_MALLOC      (USE_SSE2 && !(_M_X64 || __amd64__))
#define USE_POSIX_MEMALIGN (USE_ALTIVEC && __GLIBC__ && !__PPC64__)
