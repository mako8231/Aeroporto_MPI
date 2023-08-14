all:
	mpicc main.c -o main.out && mpirun main.out
