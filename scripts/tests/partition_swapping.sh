#!/bin/bash
executable=$1 # First arg: The executable location
output_dir=$2 # Second arg: The output directory

base="$executable --debug --rows 256 --columns 256"

echo "########### Aggregate Hierarchy ###########"
$base --targets 0 --seed 1 --partition_swaps=true --output "$output_dir/aggregate_hierarchy/true/"
echo "---"
$base --targets 0 --seed 1 --partition_swaps=false --output "$output_dir/aggregate_hierarchy/false/"

echo "########### Highest parent ###########"
$base --targets 1 --seed 1 --partition_swaps=true --output "$output_dir/highest_parent/true/"
echo "---"
$base --targets 1 --seed 1 --partition_swaps=false --output "$output_dir/highest_parent/false/"

echo "########### Aggregate Hierarchy 4c ###########"
$base --targets 2 --seed 1 --partition_swaps=true --output "$output_dir/aggregate_hierarchy_4c/true/"
echo "---"
$base --targets 2 --seed 1 --partition_swaps=false --output "$output_dir/aggregate_hierarchy_4c/false/"

echo "########### Highest parent 4c ###########"
$base --targets 3 --seed 1 --partition_swaps=true --output "$output_dir/highest_parent_4c/true/"
echo "---"
$base --targets 3 --seed 1 --partition_swaps=false --output "$output_dir/highest_parent_4c/false/"

echo "########### Partition neighbourhood ###########"
$base --targets 4 --seed 1 --partition_swaps=true --output "$output_dir/partition_neighbourhood/true/"
echo "---"
$base --targets 4 --seed 1 --partition_swaps=false --output "$output_dir/partition_neighbourhood/false/"
