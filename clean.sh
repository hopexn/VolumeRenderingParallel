#!/usr/bin/env bash
rm -rf build/*
rm -rf cmake-build-debug
rm -rf VolumeRendererServer*
rm -rf client/build
rm -rf server/build
rm -rf data/*.tmp
rm -rf data/*.blk
rm -rf tmp.zip

cd third_party/jpeg-9c
make clean
cd ../../
