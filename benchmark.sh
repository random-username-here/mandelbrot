#!/bin/bash

RUNS=128
FACTOR=1.003

mkdir -p res

for I in simple avx avx2; do
	stdbuf -o0 ./benchmark -g $I -m $RUNS -v $FACTOR  | tee ./res/$I.log
done
