#!/bin/bash

[ ! $# -ge 3 ] && echo "Usage: ./tests.sh K TestCase File" && exit 1

for i in 1 2 2a 3 3a 3b 3c 4a 4b 4c 4ci 
do
    P=$(build/index$i-main $3 -test $1 $2)
    echo "[index$i] $P"
done
