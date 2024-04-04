#!/bin/bash
executable=$1 # First arg: The executable location
shift
output_dir=$1 # Second arg: The output directory
shift
dataset_flags="$@" # All remaining flags are the data set specification. For real data, this is the config and input. For debug, this is the debug flag and dimensions.

base="$executable $dataset_flags --log_only --seed 1 --passes 5"

target_names=( "aggregate_hierarchy" "highest_parent" "aggregate_hierarchy_4c" "partition_neighbourhood" )

base_iterations=( 1000 100 50 5)
base_thresholds=( "0.001" "0.0001" "0.00001" "0.000001" "0")
iterations_changes=( "0.1" "0.5" "1" "1.5" "2")
threshold_changes=( "0.1" "0.5" "1" "2" "10" )

# Iterate over all possible combinations
for target_idx in {0..4}; do
  echo "########### ${target_names[$target_idx]} ###########"
  for iterations in "${base_iterations[@]}"; do
    for threshold in "${base_thresholds[@]}"; do
      for iterations_change in "${iterations_changes[@]}"; do
        for threshold_change in "${threshold_changes[@]}"; do
          current_output_dir="$output_dir/${target_names[$target_idx]}/it$iterations-dt$threshold-itc$iterations_change-dtc$threshold_change/"
          $base --targets "$target_idx" --max_iterations "$iterations" --min_distance_change "$threshold" --iterations_change_factor "$iterations_change" --distance_change_factor "$threshold_change" --output "$current_output_dir"
        done
      done
    done
  done
done