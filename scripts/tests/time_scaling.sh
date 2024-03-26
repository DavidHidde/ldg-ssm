#!/bin/bash
executable=$1 # First arg: The executable location
output_dir=$2 # Second arg: The output directory

base="$executable --debug --seed 1 --log_only --max_iterations 1"

target_names=( "aggregate_hierarchy" "highest_parent" "aggregate_hierarchy_4c" "highest_parent_4c" "partition_neighbourhood" )
side_lens=( 32 64 128 256 )

# Iterate over all possible combinations
for target_idx in {0..4}; do
  echo "########### ${target_names[$target_idx]} ###########"
  for side_len in "${side_lens[@]}"; do
    for repetition in {0..4}; do
      current_output_dir="$output_dir/${target_names[$target_idx]}/$side_len/$repetition/"
      $base --targets "$target_idx" --output "$current_output_dir"
    done
  done
done
