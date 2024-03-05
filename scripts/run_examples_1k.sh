# Provide the input, output and executable location as script arguments
input_dir=$1
output_dir=$2
executable=$3

# just runs a specified number of iterations (default low number for testing, does not yield refined grid)
niter=16

#
# STOCK1K
#
feat=$input_dir/stock1k.config
out=$output_dir/stock1k
mkdir -p $out

cmd_base="$executable -d $feat --outDir $out/ --distFuncType 1 --repAggregationType 4"
echo "cmd_base: $cmd_base"

$cmd_base  --termIterations $niter --noImgOut
$cmd_base --loadAssignments ${out}/qtLeafAssignment.raw.bz2 --termTime 0 --imgOutOpts "png:0,level:0,level:1,level:2"
$cmd_base --loadAssignments ${out}/qtLeafAssignment.raw.bz2 --termTime 0 --imgOutOpts "png:0,disparity:0.67"

#
# MCMC1K
#
feat=$input_dir/mcmc1k.config
out=$output_dir/mcmc1k
mkdir -p $out

cmd_base="$executable -d $feat --outDir $out/ --distFuncType 1 --repAggregationType 3"
echo "cmd_base: $cmd_base"

$cmd_base  --termIterations $niter --noImgOut
$cmd_base --loadAssignments ${out}/qtLeafAssignment.raw.bz2 --termTime 0 --imgOutOpts "png:0,level:0,level:1,level:2"
$cmd_base --loadAssignments ${out}/qtLeafAssignment.raw.bz2 --termTime 0 --imgOutOpts "png:0,disparity:0.67"

#
# CALTECH1K
#
feat=$input_dir/caltech1k_feat.config
img=$input_dir/caltech1k_img.config
out=$output_dir/caltech1k
mkdir -p $out

cmd_base="$executable -d $feat --outDir $out/ --distFuncType 2 --repAggregationType 2"
echo "cmd_base: $cmd_base"

$cmd_base  --termIterations $niter --noImgOut --writeGridAssignmentTXT ${out}/txtresult.txt
#$cmd_base --loadAssignments ${out}/qtLeafAssignment.raw.bz2 --termTime 0 --imgOutOpts "png:0,level:0,level:1,level:2" -r $img
#$cmd_base --loadAssignments ${out}/qtLeafAssignment.raw.bz2 --termTime 0 --imgOutOpts "png:0,disparity:0.67" -r $img
