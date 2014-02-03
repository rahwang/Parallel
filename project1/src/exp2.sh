touch time.txt
make clean; make
out=res2.txt
# run parallel speedup experiments
for x in 16 32 64 128 256 512 1024
do
    ./sample_m $x
    rm time.txt
    for t in 2 4 8 16 32 64
    do
	echo $x >> $out
	echo $t >> $out
	for i in `seq 0 9`
	do 
	    ./floyd_parallel $t scratch.txt >> time.txt
	done
	python std_dev.py >> $out
    done
done
