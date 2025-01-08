#include "setup.h"
#include "deoxysbc.h"
#define BPI 8
struct _prp_ctx {
    BLOCK round_keys_1 [DEOXYS_BC_128_256_NUM_ROUND_KEYS];
    BLOCK round_keys_2 [DEOXYS_BC_128_256_NUM_ROUND_KEYS];
    BLOCK round_keys_3 [DEOXYS_BC_128_256_NUM_ROUND_KEYS];
    BLOCK round_keys_h [DEOXYS_BC_128_256_NUM_ROUND_KEYS];
    BLOCK round_keys_c [DEOXYS_BC_128_256_NUM_ROUND_KEYS];

    BLOCK round_keys_d_1 [DEOXYS_BC_128_256_NUM_ROUND_KEYS];
    BLOCK round_keys_d_2 [DEOXYS_BC_128_256_NUM_ROUND_KEYS];
    BLOCK round_keys_d_3 [DEOXYS_BC_128_256_NUM_ROUND_KEYS];
};
typedef struct _prp_ctx prp_ctx;

prp_ctx* prp_allocate  (void *misc);  /* Allocate ae_ctx, set optional ptr   */
void    prp_free       (prp_ctx *ctx); /* Deallocate ae_ctx struct            */
int     prp_clear      (prp_ctx *ctx); /* Undo initialization                 */
int     prp_ctx_sizeof(void);        /* Return sizeof(ae_ctx)               */

int prp_init(prp_ctx     *ctx,
            const void *key,
            int         key_len,
            int         tweak_len);
int prp_encrypt(prp_ctx     *ctx,
            const void *pt,
            uint64_t    pt_len,
            const void *tk,
            uint64_t    tk_len,
            void       *ct, 
            int encrypt); //If possible to implement encrypt & decrypt in the same funtion with samll tweaks
int prp_decrypt(prp_ctx    *ctx,
            const void *ct,
            uint64_t        ct_len,
            const void *tk,
            uint64_t    tk_len,
            void       *pt);
