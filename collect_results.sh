#!/bin/bash
INDEX="index"
WHERE="i5"

[ ! $# -ge 1 ] && echo "Example: ./collect_results.sh data/italian-cities.txt" && exit 1

mkdir -p results
FILE=$1
F="${FILE##*/}"
TESTDIR=./test/test_cases/

for index in 1 2 2a 3 3a 3b 3c 4a 4b 4c 4ci 
do
    echo "Collecting results for $INDEX$index..."
    for test in `ls $TESTDIR`
    do
        echo "... for $TESTDIR$test ..."
        ./testK.sh $index $TESTDIR$test $FILE >> results/$INDEX$index.$WHERE.$F.$test.dat
    done
done

