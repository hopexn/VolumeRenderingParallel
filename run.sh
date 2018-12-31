#!/usr/bin/env bash
mpirun -np 1 ./VolumeRendererServer_master $1 $2 $3 $4 ../data/$5.vifo
