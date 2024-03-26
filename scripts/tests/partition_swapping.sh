#!/bin/bash
executable=$1 # First arg: The executable location
output_dir=$2 # Second arg: The output directory

base="$executable --debug --rows 256 --columns 256 --seed 1"

target_names=( "aggregate_hierarchy" "highest_parent" "aggregate_hierarchy_4c" "highest_parent_4c" "partition_neighbourhood" )

# Iterate over all possible combinations
for target_idx in {0..4}; do
  echo "########### ${target_names[$target_idx]} ###########"
  for repetition in {0..4}; do
        current_output_dir="$output_dir/${target_names[$target_idx]}/true/$repetition/"
        $base --targets "$target_idx" --partition_swaps=true --output "$current_output_dir"
        current_output_dir="$output_dir/${target_names[$target_idx]}/false/$repetition/"
        $base --targets "$target_idx" --partition_swaps=false --output "$current_output_dir"
  done
done
