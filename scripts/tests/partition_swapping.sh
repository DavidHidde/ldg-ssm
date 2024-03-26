#!/bin/bash
executable=$1 # First arg: The executable location
output_dir=$2 # Second arg: The output directory

base="$executable --debug --rows 256 --columns 256"

target_names=( "aggregate_hierarchy" "highest_parent" "aggregate_hierarchy_4c" "highest_parent_4c" "partition_neighbourhood" )
seeds=( 23 1000 789 3 756234)
log_onlies=( false true true true true )

# Iterate over all possible combinations
for target_idx in {0..4}; do
  echo "########### ${target_names[$target_idx]} ###########"
  for seed_idx in {0..4}; do
    seed="${seeds[$seed_idx]}"
    log_only="${log_onlies[$seed_idx]}"
    current_output_dir="$output_dir/${target_names[$target_idx]}/true/$seed/"
    $base --targets "$target_idx" --seed "$seed" --log_only="$log_only" --partition_swaps=true --output "$current_output_dir"
    current_output_dir="$output_dir/${target_names[$target_idx]}/false/$seed/"
    $base --targets "$target_idx" --seed "$seed" --log_only="$log_only" --partition_swaps=false --output "$current_output_dir"
  done
done
