#!/usr/bin/env bash
echo "Engine 256x256"
test.sh engine 1 12 256 256 8 8 8
test.sh engine 1 12 256 256 16 16 16
test.sh engine 1 12 256 256 32 32 32
test.sh engine 1 12 256 256 64 64 64
test.sh engine 1 12 256 256 128 128 128
test.sh engine 1 12 256 256 256 256 128
echo -e "\n\n\n\n\n"

echo "Aeurism 256x256"
test.sh aneurism 1 12 256 256 8 8 8
test.sh aneurism 1 12 256 256 16 16 16
test.sh aneurism 1 12 256 256 32 32 32
test.sh aneurism 1 12 256 256 64 64 64
test.sh aneurism 1 12 256 256 128 128 128
test.sh aneurism 1 12 256 256 256 256 256
echo -e "\n\n\n\n\n"

echo "Magnetic 256x256"
test.sh magnetic 1 12 256 256 16 16 16
test.sh magnetic 1 12 256 256 32 32 32
test.sh magnetic 1 12 256 256 64 64 64
test.sh magnetic 1 12 256 256 128 128 128
test.sh magnetic 1 12 256 256 256 256 256
test.sh magnetic 1 12 256 256 512 512 512
echo -e "\n\n\n\n\n"

echo "Christmas Tree 256x256"
test.sh christmastree 1 12 256 256 16 16 16
test.sh christmastree 1 12 256 256 32 32 32
test.sh christmastree 1 12 256 256 64 64 64
test.sh christmastree 1 12 256 256 128 128 128
test.sh christmastree 1 12 256 256 256 256 256
test.sh christmastree 1 12 256 256 512 512 512
echo -e "\n\n\n\n\n"

echo "Flower 256x256"
test.sh flower 1 12 256 256 32 32 32
test.sh flower 1 12 256 256 64 64 64
test.sh flower 1 12 256 256 128 128 128
test.sh flower 1 12 256 256 256 256 256
test.sh flower 1 12 256 256 512 512 512
test.sh flower 1 12 256 256 1024 1024 1024
echo -e "\n\n\n\n\n"

echo "Engine 512x512"
test.sh engine 1 12 512 512 8 8 8
test.sh engine 1 12 512 512 16 16 16
test.sh engine 1 12 512 512 32 32 32
test.sh engine 1 12 512 512 64 64 64
test.sh engine 1 12 512 512 128 128 128
test.sh engine 1 12 512 512 512 512 128
echo -e "\n\n\n\n\n"

echo "Aeurism 512x512"
test.sh aneurism 1 12 512 512 8 8 8
test.sh aneurism 1 12 512 512 16 16 16
test.sh aneurism 1 12 512 512 32 32 32
test.sh aneurism 1 12 512 512 64 64 64
test.sh aneurism 1 12 512 512 128 128 128
test.sh aneurism 1 12 512 512 256 256 256
echo -e "\n\n\n\n\n"

echo "Magnetic 512x512"
test.sh magnetic 1 12 512 512 16 16 16
test.sh magnetic 1 12 512 512 32 32 32
test.sh magnetic 1 12 512 512 64 64 64
test.sh magnetic 1 12 512 512 128 128 128
test.sh magnetic 1 12 512 512 256 256 256
test.sh magnetic 1 12 512 512 512 512 512
echo -e "\n\n\n\n\n"

echo "Christmas Tree 512x512"
test.sh christmastree 1 12 512 512 16 16 16
test.sh christmastree 1 12 512 512 32 32 32
test.sh christmastree 1 12 512 512 64 64 64
test.sh christmastree 1 12 512 512 128 128 128
test.sh christmastree 1 12 512 512 256 256 256
test.sh christmastree 1 12 512 512 512 512 512
echo -e "\n\n\n\n\n"

echo "Flower 512x512"
test.sh flower 1 12 512 512 32 32 32
test.sh flower 1 12 512 512 64 64 64
test.sh flower 1 12 512 512 128 128 128
test.sh flower 1 12 512 512 256 256 256
test.sh flower 1 12 512 512 512 512 512
test.sh flower 1 12 512 512 1024 1024 1024
echo -e "\n\n\n\n\n"
