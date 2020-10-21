main=bin/plain
draw=src/plane/script/visualize.py
outdir=out/

$main $outdir/out.txt
python3 $draw $outdir/out.txt $outdir/fig.png
