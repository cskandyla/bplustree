#!/bin/bash
for i in `seq 1 100`;
do
    echo $i
    #    valgrind  --leak-check=full ./abtree
    ./abtree
done    
