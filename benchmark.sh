#!/bin/bash

MEASURMENTS=128
FACTOR=1.001

RUNS="$(seq 4)"
VERSIONS="gcc-o2 clang-o2 gcc-o3 clang-o3"
VARIANTS="arrays" #"simple avx avx2 arrays"

mkdir -p res/
touch res/bench.target

for RUN in ${RUNS}; do
	for NAME in ${VERSIONS}; do
		for VAR in ${VARIANTS}; do
			echo
			echo
			echo "==> Running ${VAR} compiled with ${NAME}, run #${RUN}"
			echo
			stdbuf -o0 "./build/bench-${NAME}" -g ${VAR} -m ${MEASURMENTS} -v ${FACTOR} \
				| tee "res/${NAME}.${VAR}.${RUN}.log"
		done
	done
done
