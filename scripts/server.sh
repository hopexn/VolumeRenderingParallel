#!/usr/bin/env bash
mpirun.mpich -np 1 ./VolumeRendererServer_master $1 $2 $3 data/engine.vifo