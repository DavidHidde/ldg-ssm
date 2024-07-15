#!/bin/bash

executable=$1 # First arg: The executable location
assignment_file=$2 # Second arg: The assignment to be translated
output_dir=$3 # Third arg: The output directory

executable=$1 # First arg: The executable location
shift
assignment_file=$1 # Second arg: The assignment to be translated
shift
output_dir=$1 # Third arg: The output directory
shift
dataset_flags="$@" # All remaining flags are the data set specification

$executable \
  --input "$assignment_file" \
  --output "$output_dir"  \
  --passes 0 \
  --max_iterations 0 \
  --export=true \
  --randomize=false \
  "$dataset_flags"
