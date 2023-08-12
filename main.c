/*
	TRABALHO DE SISTEMAS DISTRIBUÍDOS:
	
	Integrantes: 
		- Masao Muraoka Neto 
		- Lucas Takashi Honda
		- Arthur Lima 

	argumentos de compilação:
		- mpicc -o main aeroporto.c && mpirun -xterm -1! -np 2 ./main
*/

#include <openmpi/mpi.h>
#include <stdio.h>
#include <stdlib.h>


#define MAX_VOOS 10
/**Aqui vai ter que verificar a quantidade de processos que o seu hardware aguenta
   Pois cada aeroporto é um processo diferente 
**/
#define MAX_AEROPORTOS 4

//Datatype para o vôo 
typedef struct voo{
	int cod_voo;
	int aeroporto_origem;
	int aeroporto_destino;
	int horario_partida;
	int horario_chegada;
	int duracao;
	int tipo_voo; //0 para decolagem, 1 para pouso
} Voo;

//Datatype para o aeroporto
typedef struct aeroporto{
	int codigo;
	int n_pousos; 
	int n_decolagens;
	Voo voos_pouso[MAX_VOOS]; //voos para pouso
	Voo voos_decolagem[MAX_VOOS]; //voos para pouso
} Aeroporto;


//Função que imprime os dados do vôo
void imprimirVoo(Voo voo){
	printf("=====================DADOS DO VOO=====================\n");
	
	printf("Código do Vôo: %d\n", voo.cod_voo);
	printf("Aeroporto de origem: %d\n", voo.aeroporto_origem);
	printf("Aeroporto de destino: %d\n", voo.aeroporto_destino);
	printf("Horário de partida: %d\n", voo.horario_partida);
	printf("Horário de chegada: %d\n", voo.horario_chegada);
	printf("Duração do Vôo: %d\n", voo.duracao);
	printf("Tipo do vôo: %d\n", voo.tipo_voo);
	
	printf("======================================================\n");
}

//Função para imprimir os dados de um aeroporto 
void imprimirAeroporto(Aeroporto aero){
	
	printf("=====================DADOS DO AEROPORTO=====================\n");
	
	printf("Código do Aeroporto: %d\n", aero.codigo);
	printf("Número de pousos: %d\n", aero.n_pousos);
	printf("Número de decolagens: %d\n", aero.n_decolagens);
	
	//iteração da quantidade de pousos:
	for (int i = 0; i< aero.n_pousos; i++) {
		imprimirVoo(aero.voos_pouso[i]);
	}

	//iteração da quantidade de decolagens: 
	for (int i = 0; i< aero.n_decolagens; i++){
		imprimirVoo(aero.voos_decolagem[i]);
	}

	printf("============================================================\n");
		
}

int main(int argc, char ** argv){
	int qtd_aeroportos, process_rank, cluster_size;
	//Lista de aeroportos 
	Aeroporto * aeroportos;
	
	//Pede para o usuário informar a quantidade de aeroportos (processos);
	printf("Insira a quantidade de aeroportos: \n");
	
	scanf("%d", &qtd_aeroportos);
	
	//verificar se os aeroportos criados não ultrapassam o limite estabelecido:
	if (qtd_aeroportos > MAX_AEROPORTOS){
		printf("Limite máximo de aeroportos estourado. Insira uma quantidade inferior ou igual a %d\n", MAX_AEROPORTOS);
		return 1;
	}

	//Aloca o vetor de aeroportos na memória 
	aeroportos = (struct aeroporto*)malloc(sizeof(aeroportos) * qtd_aeroportos);

	//Iniciando o ambiente MPI 
	MPI_Init(&argc, &argv);
	
	MPI_Comm_size(MPI_COMM_WORLD, &cluster_size);
	
	

	MPI_Comm_rank(MPI_COMM_WORLD, &process_rank);

	//Finalizar o ambiente MPI 
	MPI_Finalize();

	free(aeroportos);

	return 0;
}
