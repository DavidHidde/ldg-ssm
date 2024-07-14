#!/bin/bash

executable=$1 # First arg: The executable location
output_dir=$2 # Second arg: The output directory

$executable --debug \
 --rows 128 \
 --columns 128 \
 --passes 2 \
 --max_iterations 20 \
 --min_distance_change 0.00001 \
 --distance_function 0 \
 --parent_type 0 \
 --export