# Provide the input, output and executable location as script arguments
input_dir=$1
output_dir=$2
executable=$3

# just runs a specified number of iterations (default low number for testing, does not yield refined grid)
niter=16

#
# STOCK
#
feat=$input_dir/stock.config
out=$output_dir/stock
mkdir -p $out

cmd_base="$executable -d $feat --outDir $out/ --distFuncType 1 --repAggregationType 4 --preNormData"
echo "cmd_base: $cmd_base"

$cmd_base  --termIterations $niter --noImgOut
$cmd_base --loadAssignments ${out}/qtLeafAssignment.raw.bz2 --termTime 0 --imgOutOpts "png:0,level:4,level:5,level:6"
$cmd_base --loadAssignments ${out}/qtLeafAssignment.raw.bz2 --termTime 0 --imgOutOpts "png:0,disparity:0.75"

#
# MCMC
#
feat=$input_dir/mcmc.config
out=$output_dir/mcmc
mkdir -p $out

cmd_base="$executable -d $feat --outDir $out/ --distFuncType 1 --repAggregationType 3 --nTilesAssign 131072"
echo "cmd_base: $cmd_base"

$cmd_base  --termIterations $niter --noImgOut
$cmd_base --loadAssignments ${out}/qtLeafAssignment.raw.bz2 --termTime 0 --imgOutOpts "png:0,level:3,level:4,level:5"
$cmd_base --loadAssignments ${out}/qtLeafAssignment.raw.bz2 --termTime 0 --imgOutOpts "png:0,disparity:0.75"

#
# CALTECH
#
feat=$input_dir/caltech_feat.config
img=$input_dir/caltech_img.config
out=$output_dir/caltech
mkdir -p $out

cmd_base="$executable -d $feat --outDir $out/ --distFuncType 2 --repAggregationType 2  --nTilesAssign 10240"
echo "cmd_base: $cmd_base"

$cmd_base  --termIterations $niter --noImgOut
$cmd_base --loadAssignments ${out}/qtLeafAssignment.raw.bz2 --termTime 0 --imgOutOpts "png:0,swapXY:1,level:1,level:2" -r $img
$cmd_base --loadAssignments ${out}/qtLeafAssignment.raw.bz2 --termTime 0 --imgOutOpts "png:0,swapXY:1,disparity:0.75" -r $img
