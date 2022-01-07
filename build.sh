#!/bin/bash 

g++ -m32 -I. -o app ar0.cpp -L. -lwavelet2s

# gcc -L. -lwavelet2s  ar0.cpp -c
# gcc ar0.o
# rm ar0.o