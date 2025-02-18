#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

#include "./include/setup512.h"
#include "./include/init.h"
#include "./include/utility.h"

#define VAL_LEN 4096
#define TWK_LEN  1024
void validation_test() {
    srand(time(NULL));
    printf("Testting Input Output For Various Tweak And Msg lengths \n");
    int i, j;
    ALIGN(16) unsigned char key[16];
    for(i=0; i<16; i++) key[i] = rand() & 0xFF;

    unsigned char tk[TWK_LEN];
    unsigned char pt[VAL_LEN];
    unsigned char ct[VAL_LEN];
    unsigned char dt[VAL_LEN];
    for (i=0; i < VAL_LEN; i++) pt[i] = 'a'+(i%3);
    for (i=0; i < TWK_LEN; i++) tk[i] = 'a'+(i%3);

    prp_ctx *ctx = prp_allocate(NULL);
    prp_init(ctx, key);
    int match;
    int count = 0;
    for (i=256; i<VAL_LEN; i++) {
        for (j=0; j<TWK_LEN; j++) {
            prp_encrypt(ctx,pt,VAL_LEN,tk,TWK_LEN,ct, 1);
            prp_encrypt(ctx,ct,VAL_LEN,tk,TWK_LEN,dt, 0);
            match = memcmp(pt, dt, VAL_LEN);
            count += match;
            /* printf("Match %d %d : %d\n",i, j, 1-match); */
            if(match != 0) {
                printf("Not Match \n");
                return;
            }
        }
    }
    printf("Number of Failed Test = %d \n", count);
    free(ctx);
}
int main() {
    validation_test();
    return 0;
}

