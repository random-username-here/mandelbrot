#!/bin/bash

RUNS=128
FACTOR=1.003

mkdir -p build

for CC in gcc clang; do
	for I in simple avx avx2; do
		echo "==> Running ${I} compiled with ${CC}"
		stdbuf -o0 "./build/bench-${CC}" -g ${I} -m ${RUNS} -v ${FACTOR} | tee "build/${CC}-${I}.log"
	done
done
