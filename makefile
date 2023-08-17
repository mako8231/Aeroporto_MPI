all:
	mpicc main.c -o main.out && mpirun -xterm -1! --np 4 ./main.out
