#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

#define DATAPATH   "./timedata/%s.txt"
#define BPI 4

#include "./include/setup.h"
#include "./include/init.h"
#include "./include/timedefs.h"

void printreg(const void *a, int nrof_byte){
    int i;
    unsigned char *f = (unsigned char *)a;
    for(i=0; i < nrof_byte; i++){
        printf("%02X",(unsigned char) f[nrof_byte - 1 -i]); //uint8_t c[4+8];
    }
    printf("\n");
}
void printkey(BLOCK *key) {
    printf("----------------------------------------------------------------\n");
    for(int i=0; i<=DEOXYS_BC_128_256_NUM_ROUNDS; i++) printreg(&key[i], 16);
    printf("----------------------------------------------------------------\n");
}
void printkeys(prp_ctx     * restrict ctx) {
   printkey(ctx->round_keys_1); 
   printkey(ctx->round_keys_2); 
   printkey(ctx->round_keys_3); 
   printkey(ctx->round_keys_4); 
   printkey(ctx->round_keys_h); 
   printkey(ctx->round_keys_c); 
}

#define VAL_LEN  128
#define TWK_LEN  1024
void simple_time_test() {
    ALIGN(16) unsigned char key[16] = {0x2B,0x7E,0x15,0x16,0x28,0xAE,0xD2,0xA6,0xAB,0xF7,0x15,0x88,0x09,0xCF,0x4F,0x3C};

    ALIGN(16) unsigned char tk[TWK_LEN];
    ALIGN(16) unsigned char pt[VAL_LEN];
    ALIGN(16) unsigned char ct[VAL_LEN];
    ALIGN(16) unsigned char dt[VAL_LEN];

    for (int i=0; i < VAL_LEN; i++) pt[i] = 'a'+(i%3);
    for (int i=0; i < TWK_LEN; i++) tk[i] = 'a'+(i%3);

    prp_ctx *ctx = prp_allocate(NULL);
    int suc = prp_init(ctx, key, 16, 16);
    /* printf("Success Allocation %d \n", suc); */
    printkeys(ctx);

    prp_encrypt(ctx,pt,VAL_LEN,tk,TWK_LEN,ct, 1);
    prp_encrypt(ctx,ct,VAL_LEN,tk,TWK_LEN,dt, 0);

    for(int i=0; i< VAL_LEN/16; i++) printreg(pt + 16*i, 16);
    printf("------------------------------------------------------------------\n");
    for(int i=0; i< VAL_LEN/16; i++) printreg(ct + 16*i, 16);
    printf("------------------------------------------------------------------\n");

    int match = memcmp(pt, dt, VAL_LEN);
    printf("Decryted Value Match to the Plaintext: %d\n",1-match);
    printf("------------------------------------------------------------------\n");

//SIMPLE TIME TEST
    DO(prp_encrypt(ctx,pt,VAL_LEN,tk,TWK_LEN,ct, 1); pt[64]=ct[64];);
    printreg(ct+32,32);
    double tmpd = ((median_get())/((VAL_LEN+TWK_LEN)*(double)N));
    printf("median cycles per byte (HCTR+ Encrypt)= %lf \n\n",tmpd);

    int sum = 0;
    DO(suc = prp_init(ctx, key, 16, 16);key[5] = ((unsigned char *)(&ctx->round_keys_1[14]))[0]);
    printreg(&ctx->round_keys_1[11], 16);
    tmpd = ((median_get())/((double)N));
    printf("Sum = %d median cycles per byte (HCTR+ Encrypt)= %lf \n\n",sum, tmpd);
    prp_free(ctx);
}
void multi_len_time_test() {
    char filename[256], infostring[256], buf[256];
    getCompilerConfig(buf);
    #if PHASH == 1
    sprintf(infostring, CIPHERINFO, "hctr+phash", buf);
    #else
    sprintf(infostring, CIPHERINFO, "hctr+zhash", buf);
    #endif
    sprintf(filename,   DATAPATH, infostring);
    printf("%s \n", filename);
    printf("%s \n", infostring);
    int a = getTimeMy( infostring, filename);
    printf("a = %d \n");
}
int main() {
    simple_time_test();
    multi_len_time_test();
    return 0;
}

