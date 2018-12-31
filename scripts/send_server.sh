#!/usr/bin/env bash
../clean.sh
zip -q -r tmp.zip ./client ./common ./server ./third_party ./.gitignore ./CMakeLists.txt *.sh *.pbs
scp -r tmp.zip cadcg@10.189.141.214:/home/cadcg/hopen
