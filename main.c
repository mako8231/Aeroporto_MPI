/*
	TRABALHO DE SISTEMAS DISTRIBUÍDOS:
	
	Integrantes: 
		- Masao Muraoka Neto 
		- Lucas Takashi Honda

	argumentos de compilação (se vc estiver rodando isso no linux com o xterm instalado):
		- mpicc -o main aeroporto.c && mpirun -xterm -1! -np 2 ./main
*/

#include <openmpi/mpi.h>
#include <string.h>
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

void inicializarVoo(Voo * voo);
void inicializarAeroporto(Aeroporto * aero);
void imprimirVoo(Voo voo);
void imprimirAeroporto(Aeroporto aero);
void lerArquivo(Aeroporto * aero, const char * nome);
void atualizarRelogioLamport(RelogioLamport * relogio, int tempoNovoEvento);
void imprimirRelogioLamport(RelogioLamport relogio);
void delay(int segundos);
int processarBuffer(char buffer[20], Aeroporto * aero);
void ajustarPrioridades(Aeroporto * aero);

int main(int argc, char ** argv){
	int qtd_aeroportos, process_rank, cluster_size, t;
	//Buffer da mensagem pra enviar
	char message_buffer[MAX_AEROPORTOS][20];
	//Buffer da mensagem recebida
	char send_message_buffer[20];

	for (int i = 0; i<MAX_AEROPORTOS; i++){
		strcpy(message_buffer[i], "0 0 0 0 0 0 0");
	}
	
	//Lista de aeroportos 
	Aeroporto aeroportos[MAX_AEROPORTOS];
	RelogioLamport relogios[MAX_AEROPORTOS];

	//Iniciando o ambiente MPI
	MPI_Init(&argc, &argv);
	
	MPI_Comm_size(MPI_COMM_WORLD, &cluster_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &process_rank);
	//cada processo lê a configuração de um aeroporto
	lerArquivo(&aeroportos[process_rank], nomes[process_rank]);
	//Inicializar o relógio lógico
	relogios[process_rank].tempo = 0;

	while (TRUE){
		imprimirAeroporto(aeroportos[process_rank]);
		printf("Horário: %d\n", relogios[process_rank].tempo);
		//enviar informação do voo para os aeroportos 
		MPI_Barrier(MPI_COMM_WORLD);
		for (int i = 0; i<cluster_size; i++){
			int dummy_1, dummy_2, codigo; 
			MPI_Bcast(&message_buffer, sizeof(message_buffer), MPI_CHAR, i, MPI_COMM_WORLD);

			sscanf(message_buffer[process_rank], "%d %d %d", &dummy_1, &dummy_2, &codigo);
			/*for (int j = 0; j<cluster_size; j++){
				printf("%s", message_buffer[j]);
				printf("\n");
			}*/	
			
			if (codigo == aeroportos[process_rank].codigo){
				//Atualizando a lista do aeroporto
				int evento_tempo = processarBuffer(message_buffer[process_rank], &aeroportos[process_rank]);
				atualizarRelogioLamport(&relogios[process_rank], evento_tempo);
				ajustarPrioridades(&aeroportos[process_rank]);
			}
			//Limpar a mensagem
			strcpy(message_buffer[process_rank], "0 0 0 0 0 0 0");
		}		

		for (int i = 0; i<aeroportos[process_rank].n_decolagens; i++){
			//enviar a mensagem referente ao voo do seu tempo atual
			if (aeroportos[process_rank].voos_decolagem[i].horario_partida == relogios[process_rank].tempo){
				//Escrever a mensagem:
				imprimirVoo(aeroportos[process_rank].voos_decolagem[i]);
				printf("Enviando mensagem de voo para o aeroporto %d\n", aeroportos[process_rank].voos_decolagem[i].aeroporto_destino);
				//processamento de mensgaens
				/*
					estrutura da mensagem
					COD_VOO, AEROPORTO_ORIGEM, AEROPORTO_DESTINO, TEMPO_DECOLAGEM, TEMPO_CHEGADA, DURAÇÃO_VOO, TEMPO_DO_RELOGIO
				*/

				//processando o buffer de mensagem
				sprintf(message_buffer[aeroportos[process_rank].voos_decolagem[i].aeroporto_destino - 1], "%d %d %d %d %d %d %d", 
				aeroportos[process_rank].voos_decolagem[i].cod_voo,
				aeroportos[process_rank].voos_decolagem[i].aeroporto_origem,
				aeroportos[process_rank].voos_decolagem[i].aeroporto_destino,
				aeroportos[process_rank].voos_decolagem[i].horario_partida,
				aeroportos[process_rank].voos_decolagem[i].horario_chegada,
				aeroportos[process_rank].voos_decolagem[i].tipo_voo,
				relogios[process_rank].tempo);

				atualizarRelogioLamport(&relogios[process_rank], 0);

				printf("Mensagem enviada\n");
				
			}
		}
		delay(10000);
	}

	//Finalizar o ambiente MPI 
	MPI_Finalize();
	return 0;
}

void ajustarPrioridades(Aeroporto * aero){
	//Para cada decolagem, verificar se elas não entram em conflito com voos
	int i, j; 
	for(i = 0; i<aero->n_decolagens; i++){
		for (j = 0; j<aero->n_pousos; j++){
			if (aero->voos_decolagem[i].horario_partida == aero->voos_pouso[j].horario_chegada){
				//adiar a decolagem:
				aero->voos_decolagem[i].horario_partida + 1;
				aero->voos_decolagem[i].horario_chegada = aero->voos_decolagem[i].horario_partida + aero->voos_decolagem[i].duracao; 
			}
		}
	}
}

int processarBuffer(char buffer[20], Aeroporto * aero){
	/*
	estrutura da mensagem
	COD_VOO, AEROPORTO_ORIGEM, AEROPORTO_DESTINO, TEMPO_DECOLAGEM, TEMPO_CHEGADA, DURAÇÃO_VOO, TEMPO_DO_RELOGIO
	*/
	//incrementar o número de pousos
	
	int tempo;

	Voo novo_pouso; 
	sscanf(buffer, "%d %d %d %d %d %d %d", 
	&novo_pouso.cod_voo,
	&novo_pouso.aeroporto_origem,
	&novo_pouso.aeroporto_destino,
	&novo_pouso.horario_partida, 
	&novo_pouso.horario_chegada,
	&novo_pouso.duracao,
	&tempo);

	novo_pouso.tipo_voo = 1;

	aero->voos_pouso[aero->n_pousos] = novo_pouso;
	aero->n_pousos += 1;

	return tempo;
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

		//inserir a duração do voo
		aero->voos_pouso[i].horario_chegada = aero->voos_pouso[i].horario_partida + aero->voos_pouso[i].duracao;	
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
	
		//inserir a duração do voo
		aero->voos_decolagem[i].horario_chegada = aero->voos_decolagem[i].horario_partida + aero->voos_decolagem[i].duracao;
		aero->voos_decolagem[i].aeroporto_origem = aero->codigo;
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
		printf("Tipo do vôo: pouso\n");
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

