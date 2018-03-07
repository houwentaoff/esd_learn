#!/bin/sh
rm -rf *.so
rm -rf *.o
rm -rf test
gcc -c -Wall -Werror -fpic JzReaderAPI.c
gcc -shared -o libreader.so JzReaderAPI.o
gcc -L. -Wl,-rpath=/$PWD -Wall -o test test.c -lreader
