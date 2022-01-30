#!/bin/bash 

echo coder_outputs/e_{$1}
./app.out e $1 $1.enc
