#!/bin/bash 

sh ./build.sh
./app e images/lena_gray_512.tif encode_result.tif
./app d encode_result.tif decode_result.tif
rm encode_result.tif
