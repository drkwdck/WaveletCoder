#!/bin/bash 

sh ./build.sh
./a.out e images/lena_gray_512.tif encode_result.tif
./a.out d encode_result.tif decode_result.tif
rm encode_result.tif