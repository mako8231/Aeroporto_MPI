/*
	TRABALHO DE SISTEMAS DISTRIBUÍDOS:
	
	Integrantes: 
		- Masao Muraoka Neto 
		- Lucas Takashi Honda

	argumentos de compilação (se vc estiver rodando isso no linux com o xterm instalado):
		- mpicc -o main aeroporto.c && mpirun -xterm -1! -np 2 ./main
*/

#include <openmpi/mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_VOOS 10
/**Aqui vai ter que verificar a quantidade de processos que o seu hardware aguenta
   Pois cada aeroporto é um processo diferente 
**/
#define MAX_AEROPORTOS 4
#define TRUE 	1 
#define FALSE 	0 

char nomes[MAX_AEROPORTOS][20] = {
	"aeroporto1.txt",
	"aeroporto2.txt",
	"aeroporto3.txt",
	"aeroporto4.txt"
};

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

typedef struct relogioLogico {
	int tempo;
} RelogioLamport;

//Mensagem que será enviada de um aeroporto para outro
typedef struct mensagem {
	int remetente;  //código do aeroporto remetente (process rank)
	int destinatario; //código do aeroporto destinatário (process rank)
	int tipo_mensagem; //Tipo de mensagem, se é um pedido de pouso ou um pedido de decolagem
	int tempo_evento; //evento baseado no relógio lógico
	Voo dados_voo;
} Mensagem;

void inicializarVoo(Voo * voo);
void inicializarAeroporto(Aeroporto * aero);
void imprimirVoo(Voo voo);
void imprimirAeroporto(Aeroporto aero);
void lerArquivo(Aeroporto * aero, const char * nome);
void atualizarRelogioLamport(RelogioLamport * relogio, int tempoNovoEvento);
void imprimirRelogioLamport(RelogioLamport relogio);
void delay(int segundos);

int main(int argc, char ** argv){
	int qtd_aeroportos, process_rank, cluster_size, t;
	
	//Lista de aeroportos 
	Aeroporto aeroportos[MAX_AEROPORTOS];
	RelogioLamport relogios[MAX_AEROPORTOS];

	//Iniciando o ambiente MPI
	MPI_Init(&argc, &argv);
	
	MPI_Comm_size(MPI_COMM_WORLD, &cluster_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &process_rank);
	
	//cada processo lê a configuração de um aeroporto
	lerArquivo(&aeroportos[process_rank], nomes[process_rank]);
	//inicializar os valores padrões
	inicializarAeroporto(&aeroportos[process_rank]);

	//Inicializar o relógio lógico
	relogios[process_rank].tempo = 0;

	while (TRUE){
		printf("========Insira um voo========\n");
		
		int n_decolagens = aeroportos[process_rank].n_decolagens;
		int n_pousos = aeroportos[process_rank].n_pousos;

		//variáveis para o aeroporto
		int hora_decolagem;
		int aeroporto_destino;
		int duracao_voo;
		int horario_chegada;

		printf("Insira a hora de decolagem: tempo atual: %d\n", relogios[process_rank].tempo);
		scanf("%d",&hora_decolagem);
		fflush(stdin);
		
		printf("Insira o destino do voo, MAX AEROPORTOS: %d\n", cluster_size);
		//define o código do aeroporto de origem e destino
		aeroportos[process_rank].voos_decolagem[n_decolagens].aeroporto_origem = process_rank + 1;
		scanf("%d",&aeroporto_destino);
		fflush(stdin);

		printf("Insira a duração do voo:\n");
		scanf("%d",&duracao_voo);
		 
		horario_chegada = hora_decolagem + duracao_voo;

		fflush(stdin);

		//inserção dos valores para a struct 
		aeroportos[process_rank].voos_decolagem[n_decolagens].horario_partida = hora_decolagem;
		aeroportos[process_rank].voos_decolagem[n_decolagens].aeroporto_origem = (process_rank+1);
		aeroportos[process_rank].voos_decolagem[n_decolagens].aeroporto_destino = aeroporto_destino;
		aeroportos[process_rank].voos_decolagem[n_decolagens].duracao = duracao_voo;
		aeroportos[process_rank].voos_decolagem[n_decolagens].horario_chegada = horario_chegada;
		aeroportos[process_rank].voos_decolagem[n_decolagens].cod_voo = (process_rank+1) * 10 + n_decolagens+1;

		
		aeroportos[process_rank].n_decolagens+=1;

		imprimirAeroporto(aeroportos[process_rank]);
		imprimirRelogioLamport(relogios[process_rank]);

		
	}
	

	//Finalizar o ambiente MPI 
	MPI_Finalize();
	return 0;
}

void delay(int segundos){
	//Tempo em milissegundos 
	int msecs = 1000 * segundos;
	clock_t tempo_inicial = clock();

	while(clock() < tempo_inicial + msecs);
	
}


/*Atualiza o relógio lógico para o valor máximo entre o tempo atual para o 
tempo do novo evento somando com 1*/
void atualizarRelogioLamport(RelogioLamport * relogio, int tempoNovoEvento){
	relogio->tempo = (tempoNovoEvento > relogio->tempo) ? tempoNovoEvento + 1 : relogio->tempo + 1; 
}

void imprimirRelogioLamport(RelogioLamport relogio){
	printf("HORÁRIO ATUAL DO RELOGIO LAMPORT:\n");
	printf("TEMPO: %d\n", relogio.tempo);
}

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

	inicializarAeroporto(aero);

	sscanf(buffer, "%d %d %d", &aero->codigo, &aero->n_pousos, &aero->n_decolagens);
	//lendo os dados de pouso:
	for (i=0; i<aero->n_pousos; i++){
		aero->voos_pouso[i].tipo_voo = 1;
		fgets(buffer, sizeof(buffer), arquivo);

		sscanf(buffer, "%d %d %d %d", 
			&aero->voos_pouso[i].cod_voo, 
			&aero->voos_pouso[i].aeroporto_origem, 
			&aero->voos_pouso[i].horario_chegada, 
			&aero->voos_pouso[i].duracao);
	}

	//lendo os dados de decolagem
	for (i=0; i<aero->n_decolagens; i++){
		aero->voos_decolagem[i].tipo_voo = 0;
		fgets(buffer, sizeof(buffer), arquivo);

		sscanf(buffer, "%d %d %d %d", 
			&aero->voos_decolagem[i].cod_voo, 
			&aero->voos_decolagem[i].aeroporto_destino, 
			&aero->voos_decolagem[i].horario_partida, 
			&aero->voos_decolagem[i].duracao);
	}

	fclose(arquivo);
}

//Inicializar as variáveis de voo
void inicializarVoo(Voo * voo){
	voo->aeroporto_destino = 0;
	voo->aeroporto_origem = 0;
	voo->cod_voo = 0;
	voo->duracao = 0;
	voo->horario_chegada = 0;
	voo->horario_partida = 0;
	voo->tipo_voo = -1;
}

//Inicializar os itens do Aeroporto 
void inicializarAeroporto(Aeroporto * aero){
	for (int i = 0; i<MAX_VOOS; i++){
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
	if (voo.tipo_voo == 0){
		printf("Tipo do vôo: decolagem\n");	
	} else {
		printf("Tipo do vôo: partida\n");
	}
	
	
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

