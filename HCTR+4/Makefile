FLAGS=-O3 -maes -mavx #-march=native #-g -fsanitize=address
CC=gcc

all: deoxys hctr+phash_validation hctr+zhash_validation hctr+phash_benchmark hctr+zhash_benchmark 

deoxys: test_deoxys.c ./src/deoxysbc.c 
	$(CC) $(FLAGS) $^ -o $@

hctr+phash_validation: ./src/hctr+phash.c hctr+validation.c ./src/deoxysbc.c ./src/init.c 
	$(CC) $(FLAGS) $^ -o $@

hctr+phash_benchmark: ./src/hctr+phash.c hctr+benchmark.c ./src/deoxysbc.c ./src/init.c
	@mkdir -p timedata
	$(CC) -DPHASH=1 $(FLAGS) $^ -o $@

hctr+zhash_validation: ./src/hctr+zhash.c hctr+validation.c ./src/deoxysbc.c ./src/init.c
	$(CC) $(FLAGS) $^ -o $@

hctr+zhash_benchmark: ./src/hctr+zhash.c hctr+benchmark.c ./src/deoxysbc.c ./src/init.c
	@mkdir -p timedata
	$(CC) $(FLAGS) $^ -o $@

clean:
	rm -f deoxys hctr+phash_validation hctr+zhash_validation hctr+phash_benchmark hctr+zhash_benchmark

clean-all:
	rm -f deoxys hctr+phash_validation hctr+zhash_validation hctr+phash_benchmark hctr+zhash_benchmark
	rm -rf timedata
# DO NOT DELETE

hctr+benchmark: ./include/setup.h ./include/init.h ./include/setup.h
hctr+benchmark: ./include/deoxysbc.h ./include/timedefs.h
hctr+validation: ./include/setup.h ./include/init.h ./include/setup.h
hctr+validation: ./include/deoxysbc.h
