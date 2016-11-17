#!/bin/bash

for i in 1 2 2a 3 3a 3b 3c 4a 4b 4c 4ci 
do
    build/index$i-main $3 -test $1 $2 
done
