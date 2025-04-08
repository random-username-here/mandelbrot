#!/bin/bash

RUNS=128
FACTOR=1.003

mkdir -p res

for I in 0 1 2; do
	stdbuf -o0 ./viewer/viewer -o $I -b $RUNS -f $FACTOR  | tee ./res/$I.log
done
