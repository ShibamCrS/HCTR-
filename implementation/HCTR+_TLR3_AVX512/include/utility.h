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


void printreg(const void *a, int nrof_byte);
BLOCK Double(BLOCK b);
BLOCK gf_2_128_double_four(BLOCK Y, BLOCK *S);
BLOCK gf_2_128_double_eight(BLOCK Y, BLOCK *S);
void ctr_mode(const BLOCK *ptp, const BLOCK key[DEOXYS_BC_128_256_NUM_ROUND_KEYS], uint64_t len, BLOCK W, BLOCK Z, BLOCK *ctp);
