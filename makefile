all:
	mpicc main.c -o main.out && mpirun -xterm -1! --np 1 ./main.out
