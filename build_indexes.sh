#!/bin/bash

for i in 1 2 2a 3 3a 3b 3c 4a 4b 4c 4ci 
do
    echo "building index$i..."
    echo -e "\n.\n" | build/index$i-main $1 &> /dev/null
    echo ".......... done!"
done
