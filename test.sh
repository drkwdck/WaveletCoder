#!/bin/bash 

sh ./build.sh
./app e images/lena_gray_512.tif coder_outputs/encode_result.tif
./app d encode_result.tif coder_outputs/decode_result.tif
rm coder_outputs/encode_result.tif
