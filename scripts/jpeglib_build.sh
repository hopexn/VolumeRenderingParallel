#!/usr/bin/env bash

cd third_party/jpeg-9c
./configure --disable-shared CC='mpicc' CFLAGS='-O2'
make -j32
cd .
mkdir -p ../../libs
mv ./.libs/libjpeg.a ../../libs/
make clean
cd ../../
