/*
	TRABALHO DE SISTEMAS DISTRIBUÍDOS:
	
	Integrantes: 
		- Masao Muraoka Neto 
		- Lucas Takashi Honda

	argumentos de compilação:
		- mpicc -o main aeroporto.c && mpirun -xterm -1! -np 2 ./main
*/

#include <openmpi/mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

void inicializarVoo(Voo * voo);
void inicializarAeroporto(Aeroporto * aero);
void imprimirVoo(Voo voo);
void imprimirAeroporto(Aeroporto aero);
void lerArquivo(Aeroporto * aero, const char * nome);


void lerArquivo(Aeroporto *aero, const char *nome) {
    FILE *arquivo;
    int i, j;
	char buffer[200];

	arquivo = fopen(nome, "r");

	//Verificar se não houve erro na leitura do arquivo
	if (arquivo == NULL){
		printf("Falha ao ler arquivo\n");
		return; 
	}

	//ler a primeira linha e obtendo os dados do aeroporto
	fgets(buffer, sizeof(buffer), arquivo);
	printf("Primeira linha lida\n%s", buffer);

	inicializarAeroporto(aero);

	sscanf(buffer, "%d %d %d", &aero->codigo, &aero->n_pousos, &aero->n_decolagens);
	//lendo os dados de pouso:
	for (i=0; i<aero->n_pousos; i++){
		fgets(buffer, sizeof(buffer), arquivo);
		printf("Buffer lido: %s", buffer);
		sscanf(buffer, "%d %d %d %d", 
			&aero->voos_pouso[i].cod_voo, 
			&aero->voos_pouso[i].aeroporto_origem, 
			&aero->voos_pouso[i].horario_chegada, 
			&aero->voos_pouso[i].duracao);
	}

	//lendo os dados de decolagem
	for (i=0; i<aero->n_decolagens; i++){
		fgets(buffer, sizeof(buffer), arquivo);
		printf("Buffer lido: %s", buffer);
		sscanf(buffer, "%d %d %d %d", 
			&aero->voos_decolagem[i].cod_voo, 
			&aero->voos_decolagem[i].aeroporto_origem, 
			&aero->voos_decolagem[i].horario_chegada, 
			&aero->voos_decolagem[i].duracao);
	}

	imprimirAeroporto(*aero);
	fclose(arquivo);
}

void inicializarVoo(Voo * voo){
	voo->aeroporto_destino = 0;
	voo->aeroporto_origem = 0;
	voo->cod_voo = 0;
	voo->duracao = 0;
	voo->horario_chegada = 0;
	voo->horario_partida = 0;
	voo->tipo_voo = -1;
}

void inicializarAeroporto(Aeroporto * aero){
	for (int i = 0; i<MAX_VOOS; i++){
		printf("%d\n", i);
		inicializarVoo(&aero->voos_decolagem[i]);
		inicializarVoo(&aero->voos_pouso[i]);
	}
}


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
	Aeroporto aeroportos[10];
	
	//Pede para o usuário informar a quantidade de aeroportos (processos);
	//printf("Insira a quantidade de aeroportos: \n");
	scanf("%d", &qtd_aeroportos);
	
	//verificar se os aeroportos criados não ultrapassam o limite estabelecido:
	/*if (qtd_aeroportos > MAX_AEROPORTOS){
		printf("Limite máximo de aeroportos estourado. Insira uma quantidade inferior ou igual a %d\n", MAX_AEROPORTOS);
		return 1;
	}*/

	//Aloca o vetor de aeroportos na memória 
	lerArquivo(&aeroportos[0], "configuracao.txt");
	
	
	//Iniciando o ambiente MPI
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &cluster_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &process_rank);



	//Finalizar o ambiente MPI 
	MPI_Finalize();
	return 0;
}
