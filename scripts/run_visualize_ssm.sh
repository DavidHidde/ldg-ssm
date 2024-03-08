#!/bin/bash

# Provide the input, output and executable location as script arguments
config_dir=$1
input_dir=$2
output_dir=$3
ldg_executable=$4

# Visualize resulting data
feat=$config_dir/stock1k.config
out=$output_dir/ssm
mkdir -p $out

cmd_base="$ldg_executable -d $feat --outDir $out/ --distFuncType 1 --repAggregationType 4"
$cmd_base --loadAssignments "${input_dir}/ssm-size(2).raw.bz2" --termTime 0 --termTime 0 --imgOutOpts "png:0,level:0,level:1,level:2"
$cmd_base --loadAssignments "${input_dir}/ssm-size(2).raw.bz2" --termTime 0 --imgOutOpts "png:0,disparity:0.67"