This repository contains an optimized implementation of PHCTR+ and ZHCTR+ in C with 4 round LR (TLR4). 

To compile, we need to provide the following command:
> make

To benchmark PHCTR+:
> ./hctr+phash_benchmark

To benchmark ZHCTR+:
> ./hctr+zhash_benchmark

These executables will generate cycles per byte (CPB) for various message and tweak lengths in the directory timedata 
