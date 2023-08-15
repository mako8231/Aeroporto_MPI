all:
	mpicc main.c -o main.out && mpirun -n 1 main.out
