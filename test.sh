#!/usr/bin/env bash
mpirun -np 1 ./VolumeRendererServer_test ../data/$1.vifo $2 $3 $4 $5 $6 $7 $8
