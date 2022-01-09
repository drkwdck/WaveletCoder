#!/bin/bash 

sh ./build.sh
./app.out e images/Lena-Original-Image-512x512-pixels.png coder_outputs/encode_result.png
./app.out d coder_outputs/encode_result.png coder_outputs/decode_result.png
rm coder_outputs/encode_result.png
