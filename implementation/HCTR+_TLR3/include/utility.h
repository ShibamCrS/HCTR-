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

#define REDUCTION_POLYNOMIAL  _mm_set_epi32(0, 0, 0, 135)
#define accumulate_four(x, y) { \
    x[0] = XOR(x[0], x[1]); \
    x[2] = XOR(x[2], x[3]); \
    y    = XOR(x[0], x[2]); \
}

void printreg(const void *a, int nrof_byte);
BLOCK Double(BLOCK b);
__m128i gf_2_128_double_four(__m128i Y, __m128i S[4]);
void ctr_mode(const BLOCK *ptp, const BLOCK key[DEOXYS_BC_128_256_NUM_ROUND_KEYS], uint64_t len, BLOCK W, BLOCK Z, BLOCK *ctp);
