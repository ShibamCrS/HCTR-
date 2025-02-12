#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

#include "../include/setup512.h"
#include "../include/deoxysbc.h"
#include "../include/init.h"

int prp_init(prp_ctx *ctx, const void *mkey, int key_len, int tk_len){
    //First schedule key, which will be used to generate other keys
    BLOCK round_keys [DEOXYS_BC_128_256_NUM_ROUND_KEYS];
    DEOXYS_128_256_setup_key(mkey, round_keys);
    
    __m512i round_keys4[DEOXYS_BC_128_256_NUM_ROUND_KEYS];
    for(int i=0; i<=DEOXYS_BC_128_256_NUM_ROUNDS; i++) 
        round_keys4[i] = _mm512_broadcast_i32x4(round_keys[i]);

    //At first derive 4 keys in parallel
    BLOCK States[4], S1, S2, T, t;
    T = FOUR;
    BLOCK4 RT[8];
    RT[0] = _mm512_broadcast_i32x4(T);
    for(int i=1; i<8; i++){ //UPDATE_TWEAK
        RT[i] = PERMUTE_512(RT[i-1]);
    }
    
    __m512i state = CTR4321;
    /* printreg_init(&state, 64); */
    DEOXYS( state, round_keys4, RT )
    
    States[0] = _mm512_extracti32x4_epi32(state, 0);
    States[1] = _mm512_extracti32x4_epi32(state, 1);
    States[2] = _mm512_extracti32x4_epi32(state, 2);
    States[3] = _mm512_extracti32x4_epi32(state, 3);

    DEOXYS_128_256_setup_key((unsigned char *)(&States[0]),  ctx->round_keys_1);
    DEOXYS_128_256_setup_key((unsigned char *)(&States[1]),  ctx->round_keys_2);
    DEOXYS_128_256_setup_key((unsigned char *)(&States[2]),  ctx->round_keys_3);
    DEOXYS_128_256_setup_key((unsigned char *)(&States[3]),  ctx->round_keys_h);

    DEOXYS_128_256_setup_key_decryption(ctx->round_keys_d_1, ctx->round_keys_1);
    DEOXYS_128_256_setup_key_decryption(ctx->round_keys_d_2, ctx->round_keys_2);
    DEOXYS_128_256_setup_key_decryption(ctx->round_keys_d_3, ctx->round_keys_3);

    //Now derive k_c
    S1 = _mm_set_epi32(0,0,0,5);
    T = FOUR;
    TAES(S1, round_keys, T, t);
    DEOXYS_128_256_setup_key((unsigned char *)(&S1),  ctx->round_keys_c);

    for(int i=0; i<=DEOXYS_BC_128_256_NUM_ROUNDS; i++){
        ctx->round_keys_h_512[i] = _mm512_broadcast_i64x2(ctx->round_keys_h[i]);
        ctx->round_keys_c_512[i] = _mm512_broadcast_i64x2(ctx->round_keys_c[i]);
    }

    return 1;
}
prp_ctx* prp_allocate(void *misc)
{
    void *p;
    (void) misc;                     /* misc unused in this implementation */
    #if USE_MM_MALLOC
        p = _mm_malloc(sizeof(prp_ctx),16);
    #elif USE_POSIX_MEMALIGN
        if (posix_memalign(&p,16,sizeof(prp_ctx)) != 0) p = NULL;
    #else
        p = malloc(sizeof(prp_ctx));
    #endif
    return (prp_ctx *)p;
}

void prp_free(prp_ctx *ctx)
{
    #if USE_MM_MALLOC
        _mm_free(ctx);
    #else
        free(ctx);
    #endif
}

int prp_clear (prp_ctx *ctx) /* Zero prp_ctx and undo initialization          */
{
    memset(ctx, 0, sizeof(prp_ctx));
    return 1;
}
int prp_ctx_sizeof(void) { return (int) sizeof(prp_ctx); }


