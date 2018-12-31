#!/usr/bin/env bash
../clean.sh
zip -q -r tmp.zip ./client ./common ./server ./third_party ./.gitignore ./CMakeLists.txt *.sh *.pbs
scp -r tmp.zip yanyuyu@10.76.6.200:/public/home/yanyuyu/hopen/
