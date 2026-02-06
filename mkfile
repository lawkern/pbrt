compile:V:
   mkdir -p build
   c99 -o build/pbrt main.c

run:V:
   ./build/pbrt
