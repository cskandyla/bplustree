#!/bin/bash
for i in `seq 1 1000`;
do
    echo $i
    valgrind --leak-check=full --show-leak-kinds=all  ./abtree
done    