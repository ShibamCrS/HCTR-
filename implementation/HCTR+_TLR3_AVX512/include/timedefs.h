#include <stdio.h>
#define TIME_TEST 1 /*if measuring time then 1 else 0 */

#define CACHE_WARM_ITER 1024

#ifndef MAX_ITER
	#define MAX_ITER 1024
#endif

#define M 16

#define N 128

#if __INTEL_COMPILER
  #define STAMP ((unsigned)__rdtsc())
#elif (__GNUC__ && (__x86_64__ || __amd64__ || __i386__))
  #define STAMP ({unsigned res; __asm__ __volatile__ ("rdtsc" : "=a"((unsigned)res) : : "edx"); res;})
#elif (_M_IX86)
  #include <intrin.h>
  #pragma intrinsic(__rdtsc)
  #define STAMP ((unsigned)__rdtsc())
#else
  #error -- Architechture not supported!
#endif



/* #define STAMP ({unsigned res; __asm__ __volatile__ ("rdtsc" : "=a"(res) : : "edx"); res;}) /1* Time stamp *1/ */

#define DO(x) do { \
int i,j; \
for (i = 0; i < M; i++) { \
unsigned c2, c1;\
for(j=0;j<CACHE_WARM_ITER;j++) {x;}\
c1 = STAMP;\
for (j = 1; j <= N; j++) { x; }\
c1 = STAMP - c1;\
median_next(c1);\
} } while (0)


#if (TIME_TEST == 1)
	unsigned values[MAX_ITER];
	int num_values = 0;

	int comp(const void *x, const void *y) {return *(unsigned *)x - *(unsigned *)y; }
	
	void median_next(unsigned x) {values[num_values++] = x; }
	
	unsigned median_get(void) {
		unsigned res;
		/*for (res = 0; res < num_values; res++)
		//   printf("%d ", values[res]);
		//printf("\n");*/
		qsort(values, num_values, sizeof(unsigned), comp);
		res = values[num_values/2];
		num_values = 0;
		return res;
	}

	void median_print(void) {
		int res;
		qsort(values, num_values, sizeof(unsigned), comp);
		for (res = 0; res < num_values; res++)
		printf("%d ", values[res]);
		printf("\n");
	}

void getCompilerConfig(char *outp) {
    #if __INTEL_COMPILER
        outp += sprintf(outp, "__Intel_C_%d.%d.%d_",
            (__ICC/100), ((__ICC/10)%10), (__ICC%10));
    #elif _MSC_VER
        outp += sprintf(outp, "__Microsoft_C_%d.%d_",
            (_MSC_VER/100), (_MSC_VER%100));
    #elif __clang_major__
        outp += sprintf(outp, "__Clang_C_%d.%d.%d ",
            __clang_major__, __clang_minor__, __clang_patchlevel__);
    #elif __clang__
        outp += sprintf(outp, "__Clang_C_1.x_");
    #elif __GNUC__
        outp += sprintf(outp, "__GNU_C_%d.%d.%d_",
            __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
    #endif
    
    #if __x86_64__ || _M_X64
    outp += sprintf(outp, "x86_64");
    #elif __i386__ || _M_IX86
    outp += sprintf(outp, "x86_32");
    #elif __ARM_ARCH_7__ || __ARM_ARCH_7A__ || __ARM_ARCH_7R__ || __ARM_ARCH_7M__
    outp += sprintf(outp, "ARMv7");
    #elif __ARM__ || __ARMEL__
    outp += sprintf(outp, "ARMv5");
    #elif (__MIPS__ || __MIPSEL__) && __LP64__
    outp += sprintf(outp, "MIPS64");
    #elif __MIPS__ || __MIPSEL__
    outp += sprintf(outp, "MIPS32");
    #elif __ppc64__
    outp += sprintf(outp, "PPC64");
    #elif __ppc__
    outp += sprintf(outp, "PPC32");
    #elif __sparc__ && __LP64__
    outp += sprintf(outp, "SPARC64");
    #elif __sparc__
    outp += sprintf(outp, "SPARC32");
    #endif
}
int getTime(char *infoString, char *filename) {
    /* Allocate locals */
    ALIGN(16) char pt[65536] = {0};
    ALIGN(16) unsigned char tk[512];
    ALIGN(16) unsigned char key[] = "abcdefghijklmnop";
    char outbuf[MAX_ITER*15+1024];
    
    int iter_list[2048]; /* Populate w/ test lengths, -1 terminated */
    prp_ctx *ctx = prp_allocate(NULL);
    char *outp = outbuf;
    int i, j, len, tk_len = 512;
    double Hz;
    double ipi=0, tmpd;
    
    /* populate iter_list, terminate list with negative number */
    for (i=0; i<MAX_ITER; ++i)
        iter_list[i] = i+1;
    if (MAX_ITER < 44) iter_list[i++] = 44;
    if (MAX_ITER < 552) iter_list[i++] = 552;
    if (MAX_ITER < 576) iter_list[i++] = 576;
    if (MAX_ITER < 1500) iter_list[i++] = 1500;
    if (MAX_ITER < 2048) iter_list[i++] = 2048;
    if (MAX_ITER < 4096) iter_list[i++] = 4096;
    if (MAX_ITER < 8192) iter_list[i++] = 8192;
    if (MAX_ITER < 16384) iter_list[i++] = 16384;
    if (MAX_ITER < 32768) iter_list[i++] = 32768;
    if (MAX_ITER < 65536) iter_list[i++] = 65536;
    iter_list[i] = -1;
    
    FILE *fp = fopen(filename, "w");
    char str_time[25];
    time_t tmp_time = time(NULL);
    struct tm *tp = localtime(&tmp_time);
    strftime(str_time, sizeof(str_time), "%F %R", tp);
    
    outp += sprintf(outp, "%s ", infoString);
    /* getCompilerConfig(outp); */
    outp += sprintf(outp, " : Run Date/Time %s\n\n",str_time);
    
    int sum = 0, suc = 0;
    //get AE initialization Time
    outp += sprintf(outp, "Context Size: %d bytes\n", prp_ctx_sizeof());
    DO(prp_init(ctx, key, 16, 16));
    num_values = 0;
    DO(suc = prp_init(ctx, key, 16, 16);key[5] = ((unsigned char *)(&ctx->round_keys_1[    14]))[0]);
    printf("Key setup: %6.2f cycles\n\n", ((median_get())/(double)N));
    outp += sprintf(outp, "Key setup: %6.2f cycles\n\n", ((median_get())/(double)N));

     /*
     * Get times over different lengths
     */
    i=127;
    len = iter_list[i];
    while (len >= 0) {
        tk[tk_len - 2] = 0;
        DO(prp_encrypt(ctx, pt, len, tk, tk_len, pt, 1); tk[tk_len - 2] += 1);
        tmpd = ((median_get())/((len+tk_len)*(double)N));
        outp += sprintf(outp, "%5d  %6.2f\n", len, tmpd);
        if (len==44) {
            ipi += 0.05 * tmpd;
        } else if (len==552) {
            ipi += 0.15 * tmpd;
        } else if (len==576) {
            ipi += 0.2 * tmpd;
        } else if (len==1500) {
            ipi += 0.6 * tmpd;
        }

        ++i;
        len = iter_list[i];
    }
    outp += sprintf(outp, "ipi %.2f\n", ipi);
    if (fp) {
        fprintf(fp, "%s", outbuf);
        fclose(fp);
    } else
        fprintf(stdout, "%s", outbuf);
    prp_free(ctx);
    return ((pt[0]==12) && (pt[10]==34) && (pt[20]==56) && (pt[30]==78));
}

#define PT_LEN 65536
#define TK_LEN 65536
#define EXP 16 //16 diff length of msgs
int getTimeMy(char *infoString, char *filename) {
    /* Allocate locals */
    ALIGN(16) char pt[65536] = {0};
    ALIGN(16) unsigned char tk[65536];  // = "abcdefghijklmnopabcdefghijklmnop";  //2n bit tweak
    ALIGN(16) unsigned char key[] = "abcdefghijklmnop";
    char outbuf[MAX_ITER*15+1024];
    
    prp_ctx *ctx = prp_allocate(NULL);
    char *outp = outbuf;
    int i, j, len, twk_len;
    double Hz;
    double ipi=0, tmpd;
    
    /* populate iter_list, terminate list with negative number */
    int msg_len_list[EXP] = {128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536, -1}; 
    int twk_len_list[EXP] = {0, 16, 32, 64, 128, 256, 512, -1}; 

    FILE *fp = fopen(filename, "w");
    char str_time[25];
    time_t tmp_time = time(NULL);
    struct tm *tp = localtime(&tmp_time);
    strftime(str_time, sizeof(str_time), "%F %R", tp);
    
    outp += sprintf(outp, "%s ", infoString);
    /* getCompilerConfig(outp); */
    outp += sprintf(outp, " : Run Date/Time %s\n\n",str_time);
    
    int sum = 0, suc = 0;
    //get AE initialization Time
    outp += sprintf(outp, "Context Size: %d bytes\n", prp_ctx_sizeof());
    DO(prp_init(ctx, key, 16, 16));
    num_values = 0;
    DO(suc = prp_init(ctx, key, 16, 16);key[5] = ((unsigned char *)(&ctx->round_keys_1[    14]))[0]);
    printf("Key setup: %6.2f cycles\n\n", ((median_get())/(double)N));
    outp += sprintf(outp, "Key setup: %6.2f cycles\n\n", ((median_get())/(double)N));

     /*
     * Get times over different lengths
     */
    
    outp += sprintf(outp,        "MsgLen TwkLen CPB   \n");
    i = 0;
    len = msg_len_list[i];
    while (len >= 0) {
        j=0;
        twk_len = twk_len_list[j];
        while(twk_len >= 0) {
            tk[twk_len - 2] = 0;
            DO(prp_encrypt(ctx, pt, len, tk, twk_len, pt, 1); tk[twk_len - 2] += 1);
            tmpd = ((median_get())/((double)(len+twk_len)*(double)N));
            outp += sprintf(outp, "%5d %5d  %6.2f\n", len, twk_len, tmpd);
            ++j;
            twk_len = twk_len_list[j];
        }
        ++i;
        len = msg_len_list[i];
    }
    if (fp) {
        fprintf(fp, "%s", outbuf);
        fclose(fp);
    } else
        fprintf(stdout, "%s", outbuf);
    prp_free(ctx);
    return ((pt[0]==12) && (pt[10]==34) && (pt[20]==56) && (pt[30]==78));
}
#endif
