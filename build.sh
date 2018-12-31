#!/usr/bin/env bash
cd ispc
ispc Renderer.ispc --target=avx2-i32x8 -o Renderer.o -h Renderer.h
cd ..

mkdir -p build
cd build
cmake ..
make -j4
cd ..
