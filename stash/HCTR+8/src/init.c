#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

#include "../include/setup.h"
#include "../include/deoxysbc.h"
#include "../include/init.h"

int prp_init(prp_ctx *ctx, const void *mkey, int key_len, int tk_len){
    //First schedule key, which will be used to generate other keys
    BLOCK round_keys [DEOXYS_BC_128_256_NUM_ROUND_KEYS];
    DEOXYS_128_256_setup_key(mkey, round_keys);
    
    //At first derive 4 keys in parallel
    BLOCK RT[8][4], States[4], S1, S2, T, t;
    BLOCK ctr = ZERO();
    RT[0][0] = ADD_ONE(ctr);
    RT[0][1] = ADD_ONE(ctr);
    RT[0][2] = ADD_ONE(ctr);
    RT[0][3] = ADD_ONE(ctr);

    for(int i=1; i<8; i++){ //UPDATE_TWEAK
        RT[i][0] = RT[i-1][0];
        RT[i][1] = RT[i-1][1];
        RT[i][2] = RT[i-1][2];
        RT[i][3] = RT[i-1][3];
    }
    States[0] = States[1] = States[2] = States[3] = ZERO();
    DEOXYS( States, round_keys, RT )

    DEOXYS_128_256_setup_key((unsigned char *)(&States[0]),  ctx->round_keys_1);
    DEOXYS_128_256_setup_key((unsigned char *)(&States[1]),  ctx->round_keys_2);
    DEOXYS_128_256_setup_key((unsigned char *)(&States[2]),  ctx->round_keys_3);
    DEOXYS_128_256_setup_key((unsigned char *)(&States[3]),  ctx->round_keys_4);

    DEOXYS_128_256_setup_key_decryption(ctx->round_keys_d_1, ctx->round_keys_1);
    DEOXYS_128_256_setup_key_decryption(ctx->round_keys_d_2, ctx->round_keys_2);
    DEOXYS_128_256_setup_key_decryption(ctx->round_keys_d_3, ctx->round_keys_3);
    DEOXYS_128_256_setup_key_decryption(ctx->round_keys_d_4, ctx->round_keys_4);

    //Now derive k_h and k_c
    S1 = S2 = ZERO();
    T = ADD_ONE(ctr);
    TAES(S1, round_keys, T, t);
    T = ADD_ONE(ctr);
    TAES(S2, round_keys, T, t);
    DEOXYS_128_256_setup_key((unsigned char *)(&S1),  ctx->round_keys_h);
    DEOXYS_128_256_setup_key((unsigned char *)(&S2),  ctx->round_keys_c);
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


