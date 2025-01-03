#include <stdint.h>
#include <tmmintrin.h>        /* SSSE3 instructions              */
#include <xmmintrin.h>        /* SSE instructions and _mm_malloc */
#include <emmintrin.h>        /* SSE2 instructions               */
#include <wmmintrin.h>

#define CIPHERINFO "%s_AES_NI_%s" //ex: ctr_AES_NI__GNU_C_12.2.0_x86_64 
#define ENC(a,b)       _mm_aesenc_si128(a,b)
#define DEC(a,b)       _mm_aesdec_si128(a,b)
#define DECLAST(a,b)   _mm_aesdeclast_si128(a,b)
#define MCINV(a)       _mm_aesimc_si128(a)
#define XOR(a,b)       _mm_xor_si128(a,b)
#define AND(a,b)       _mm_and_si128(a,b)
#define OR(a,b)        _mm_or_si128(a,b)
#define ADD_ONE(b)     _mm_add_epi32(b,_mm_set_epi32(0,0,0,1))
#define CTR1           _mm_set_epi32(0,0,0,1)

#define ZERO()         _mm_setzero_si128()
#define ONE     _mm_set_epi8( 0b00100000, 0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 )
#define TWO     _mm_set_epi8( 0b01000000, 0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 )
#define THREE   _mm_set_epi8( 0b01100000, 0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 )
#define FOUR    _mm_set_epi8( 0b10000000, 0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 )
#define MASKD   _mm_set_epi8( 0b00011111, 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF )
#define TRUNC(a, b) XOR(AND(a, MASKD), b)
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
/* These determine how to allocate 16-byte aligned vectors, if needed.     */
#define USE_MM_MALLOC      (USE_SSE2 && !(_M_X64 || __amd64__))
#define USE_POSIX_MEMALIGN (USE_ALTIVEC && __GLIBC__ && !__PPC64__)
