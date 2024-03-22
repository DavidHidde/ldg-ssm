#!/bin/bash
executable=$1 # First arg: The executable location
output_dir=$2 # Second arg: The output directory

base="$executable --debug --rows 256 --columns 256 --targets 4 --seed 1 --log_only"

echo "## 1 ##"
$base --cores 1 --output "$output_dir/1/"
echo "## 2 ##"
$base --cores 2 --output "$output_dir/2/"
echo "## 4 ##"
$base --cores 4 --output "$output_dir/4/"
echo "## 8 ##"
$base --cores 8 --output "$output_dir/8/"
echo "## 16 ##"
$base --cores 16 --output "$output_dir/16/"
echo "## 32 ##"
$base --cores 32 --output "$output_dir/32/"
echo "## 64 ##"
$base --cores 64 --output "$output_dir/64/"
echo "## 128 ##"
$base --cores 128 --output "$output_dir/128/"
