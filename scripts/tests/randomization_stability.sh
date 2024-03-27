#!/bin/bash
executable=$1 # First arg: The executable location
output_dir=$2 # Second arg: The output directory

base="$executable --debug --rows 256 --columns 256"

target_names=( "aggregate_hierarchy" "highest_parent" "aggregate_hierarchy_4c" "highest_parent_4c" "partition_neighbourhood" )
seeds=( 1 5 23 75 456 666 3492 7890 90000 123456 )

# Iterate over all possible combinations
for target_idx in {0..4}; do
  echo "########### ${target_names[$target_idx]} ###########"
  for seed in "${seeds[@]}"; do
    current_output_dir="$output_dir/${target_names[$target_idx]}/$seed/"
    $base --targets "$target_idx" --seed "$seed" --output "$current_output_dir"
  done
done
