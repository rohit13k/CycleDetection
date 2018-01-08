#location of data folder
DATA_DIR=/home/rohit/work/data
CLEANUP=100000

for WINDOW in 1 10;
do
	for INPUT_FILE in sma-a facebook-wosn-wall;
	do
	# For Naive Algorithm
		OUTPUT_FILE=$INPUT_FILE-root-$WINDOW-naive.csv
		echo `date +%Y/%m/%d-%H:%M`  started computation of $OUTPUT_FILE >> CycleDetection.log
		./CycleDetection -i $DATA_DIR/$INPUT_FILE$INPUT_EXTENSION -o $DATA_DIR/$OUTPUT_FILE -w $WINDOW -p $CLEANUP -l $CYCLE -r 0 -a 0 -z 1 >> $INPUT_FILE-$WINDOW-Cycle-Naive.log
		echo `date +%Y/%m/%d-%H:%M` finished computation of $OUTPUT_FILE >> CycleDetection.log
		OUTPUT_FILE=$INPUT_FILE-root-$WINDOW.csv
	# Source Detection Phase: For Finding root nodes using Exact set and with compression 	
	    echo `date +%Y/%m/%d-%H:%M`  started computation of $OUTPUT_FILE >> CycleDetection.log
		./CycleDetection -i $DATA_DIR/$INPUT_FILE$INPUT_EXTENSION -o $DATA_DIR/$OUTPUT_FILE -w $WINDOW -p $CLEANUP -l $CYCLE -r 0 -a 9 -z 1 >> $INPUT_FILE-$WINDOW-Cycle-root.log
		echo `date +%Y/%m/%d-%H:%M` finished computation of $OUTPUT_FILE >> CycleDetection.log
		OUTPUT_FILE=$INPUT_FILE-root-$WINDOW-compressed.csv
	# Finding cycle without bundle 	
		echo `date +%Y/%m/%d-%H:%M`  started computation of $OUTPUT_FILE >> CycleDetection.log
           	./CycleDetection -i $DATA_DIR/$INPUT_FILE$INPUT_EXTENSION -o $DATA_DIR/$OUTPUT_FILE -w $WINDOW -p $CLEANUP -l $CYCLE -r 0 -a 3 -b 0 -z 1 >> $INPUT_FILE-$WINDOW-Cycle.log
		echo `date +%Y/%m/%d-%H:%M` finished computation of $OUTPUT_FILE >> CycleDetection.log
		mv $DATA_DIR/$INPUT_FILE-root-$WINDOW-compressed.cycle $DATA_DIR/$INPUT_FILE-root-$WINDOW-compressed_nonbundle.cycle
	# Finding cycle with bundle	
		echo `date +%Y/%m/%d-%H:%M`  started computation of $OUTPUT_FILE >> CycleDetection.log
		./CycleDetection -i $DATA_DIR/$INPUT_FILE$INPUT_EXTENSION -o $DATA_DIR/$OUTPUT_FILE -w $WINDOW -p $CLEANUP -l $CYCLE -r 0 -a 3 -b 1 -z 1 >> $INPUT_FILE-$WINDOW-Cycle.log
       	echo `date +%Y/%m/%d-%H:%M` finished computation of $OUTPUT_FILE >> CycleDetection.log
		OUTPUT_FILE=$INPUT_FILE-root-$WINDOW-bloom.csv
	# Finding cycle with bloom filter and bundle	
		echo `date +%Y/%m/%d-%H:%M`  started computation of $OUTPUT_FILE >> CycleDetection.log
		./CycleDetection -i $DATA_DIR/$INPUT_FILE$INPUT_EXTENSION -o $DATA_DIR/$OUTPUT_FILE -w $WINDOW -p $CLEANUP -l $CYCLE -r 0 -a 7 -b 0 -z 1 >> $INPUT_FILE-$WINDOW-Cycle-bloom.log
		echo `date +%Y/%m/%d-%H:%M` finished computation of $OUTPUT_FILE >> CycleDetection.log
	done
done

# For finding cycles in twitter dataset with reverse edge direction using bloom
for WINDOW in 1 10;
do
	for INPUT_FILE in higgs-activity_time;
	do
		OUTPUT_FILE=$INPUT_FILE-root-$WINDOW-bloom.csv
		echo `date +%Y/%m/%d-%H:%M`  started computation of $OUTPUT_FILE >> CycleDetection.log
		./CycleDetection -i $DATA_DIR/$INPUT_FILE$INPUT_EXTENSION -o $DATA_DIR/$OUTPUT_FILE -w $WINDOW -p $CLEANUP -l $CYCLE -r 1 -a 7 -b 0 -z 1 >> $INPUT_FILE-$WINDOW-Cycle-bloom.log
		echo `date +%Y/%m/%d-%H:%M` finished computation of $OUTPUT_FILE >> CycleDetection.log
	done
done



