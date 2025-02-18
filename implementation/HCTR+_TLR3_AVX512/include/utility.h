void printreg(const void *a, int nrof_byte);
BLOCK Double(BLOCK b);
BLOCK gf_2_128_double_four(BLOCK Y, BLOCK *S);
BLOCK gf_2_128_double_eight(BLOCK Y, BLOCK *S);
BLOCK gf_2_128_double_16(BLOCK Y, BLOCK *S);
void ctr_mode(const BLOCK *ptp, const BLOCK4 key4[DEOXYS_BC_128_256_NUM_ROUND_KEYS], const BLOCK key[DEOXYS_BC_128_256_NUM_ROUND_KEYS], uint64_t len, BLOCK W, BLOCK Z, BLOCK *ctp);
void xorpp_mode(const BLOCK *ptp, const BLOCK4 key4[DEOXYS_BC_128_256_NUM_ROUND_KEYS], const BLOCK key[DEOXYS_BC_128_256_NUM_ROUND_KEYS], uint64_t len, BLOCK W, BLOCK Z, BLOCK *ctp);
