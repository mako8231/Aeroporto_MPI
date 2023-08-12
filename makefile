all:
	mpicc -o main aeroporto.c && mpirun -xterm -1! -np 2 ./main
