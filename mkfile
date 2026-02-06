CC=c99
CFLAGS=-Wall -ggdb -O0

compile:V:
   mkdir -p build
   $CC -o build/multicore.o -c multicore_posix.c $CFLAGS -D_POSIX_C_SOURCE=200112L -lpthread
   $CC -o build/pbrt main.c build/multicore.o $CFLAGS

run:V:
   ./build/pbrt
