touch time.txt
make
out=res1.txt

echo "" > $out

# run parallel overhead experiments
for x in 16 32 64 128 256 512 1024
do
    echo $x >> $out
    ./sample_m $x
    rm time.txt
    for i in `seq 0 9`
    do 
	./floyd_serial scratch.txt >> time.txt
    done
    python std_dev.py >> $out
    rm time.txt
    for i in `seq 0 9`
    do 
	./floyd_parallel 1 scratch.txt >> time.txt
    done
    python std_dev.py >> $out
done
