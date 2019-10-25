#!/bin/bash

# echo mpirun -np $1 --oversubscribe floyd $2
mpirun --hostfile $3 -np $1 --oversubscribe floyd $2 $4
