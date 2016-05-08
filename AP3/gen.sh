#!/bin/bash

for dim in 5 10 20 30 40 50
do
  echo $dim
  for seed in `seq 1 10`
  do
    echo $seed
    ./map $seed $dim
  done
done
