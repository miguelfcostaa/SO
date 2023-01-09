#include <stdio.h>
#include <ctype.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>
#include <semaphore.h>

#define UNIXSTR_PATH "/tmp/s.2079120"
#define MAXLINE 1024    //tamanho maximo do buffer
#define SIZE_TASKS 1000000
#define TRUE 1
#define FALSE 0

//CORES
#define STOP "\x1B[0m"
#define WHITE "\033[0;37m"
#define PURPLE "\x1B[35m"
#define YELLOW "\x1B[33m"
#define BLUE "\x1B[34m"
#define CYAN "\x1B[36m"
#define RED "\x1B[31m"
#define GREEN "\x1B[32m"


//Estado pessoa
#define ESPERA 0
#define ENTROU 1
#define COMENDO 2
#define COMA 3
#define MULHER 4
#define HOMEM 5


//Zona
#define JAM 0
#define SMOKING_AREA 1
#define PADARIA 2


struct configuracao {
    int tamanhoDiscoteca;
    int tempoLimiteSimulacao;
    int tamanhoMaxFila1;
    int tamanhoMaxFila2;
    int tamanhoMaxJam;
    int tamanhoMaxSmokingArea;
    int tamanhoMaxPadaria;
    int tempoMedioChegada;
    int tempoMedioEspera;
    int tempoMaxEspera;
    int tempoLevarSoro;
    int tempoComer;
    int tempoFumar;
    float probSerVIP;
    float probEntrarComa;
    float probMorrerComa;
    float probIr;
    float probMudarZona;
};

struct pessoa {
    int id;
    int sexualidade; //MULHER - 0 | HOMEM - 1
    int fila;
    int zona; //0 - Zona A | 1 - Zona B | 2 - Padaria 
    int vip; 
    int desistir;
    int nPessoasAFrenteDesistir;
    int tempoMaxEsperaP;
    int estado; // 0 = espera | 1 = coma
};


struct Fila1 {
    int nPessoasFila1;
};

struct Fila2 {
    int nPessoasFila2;
};


//simulador.c
int criaSocket();
void enviaInformacao(int sockfd, int pessoa_id, int tempoMedido, int estado, int acontecimento, int zona);
int probabilidade (float valor);
int randomNumber(int max, int min);
char *defineTipoPessoa(struct pessoa *people);
struct pessoa criaPessoa();
void FilaDeEspera(struct pessoa *people);
void Pessoa(void *ptr);
int readConfiguracao(char ficheiro[]);
int simulacao(char* configFile);
void semaforosTrincos();


//monitor.c
void socketservidor();
void recebeInformacao(int newsockfd);
void limpaFeedback();
void escreveFeedback();
void imprimeFeedback();
