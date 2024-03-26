#!/bin/bash
executable=$1 # First arg: The executable location
output_dir=$2 # Second arg: The output directory

base="$executable --debug --rows 256 --columns 256 --targets 4 --seed 1 --log_only"

num_cores=( 1 2 4 8 16 32 64 128)

# Iterate over all possible combinations
for cores in "${num_cores[@]}"; do
    for repetition in {0..4}; do
      current_output_dir="$output_dir/$cores/$repetition/"
      $base --cores "$cores" --output "$current_output_dir"
    done
done
