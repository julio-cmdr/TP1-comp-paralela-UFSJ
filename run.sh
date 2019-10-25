#!/bin/bash

# echo mpirun -np $1 --oversubscribe floyd $2
mpirun -np $1 --oversubscribe floyd $2 $3
