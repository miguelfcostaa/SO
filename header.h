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
#define SIZE_TASKS 100000
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
#define COMA 2
#define MULHER 3
#define HOMEM 4


//TESTE DE COMA
#define NAO_FEZ_TESTE 0
#define LEVANDO_SORO 1
#define MORREU 2
#define SOBREVIVEU 3

//Zona
#define ZONA_A 0
#define ZONA_B 1
#define PADARIA 2

//TEMPO 
#define MINUTO 60 //1 minuto = 60 segundos
#define HORA 60 * 60 //1 hora = 3600 segundos


struct configuracao {
    int tamanhoDiscoteca;
    int tempoLimiteSimulacao;
    int tamanhoMaxFila1;
    int tamanhoMaxFila2;
    int tamanhoMaxZonaA;
    int tamanhoMaxZonaB;
    int tamanhoMaxPadaria;
    int tempoMedioChegada;
    int tempoMedioEspera;
    int tempoMaxEspera;
    int tempoLevarSoro;
    float probSerVIP;
    float probDesistir;
    float probSerMulher;
    float probSerHomem;
    float probEntrarComa;
    float probMorrerComa;
    float probIrComer;
};

struct pessoa {
    int id;
    int sexualidade; //MULHER - 0 | HOMEM - 1
    int fila;
    int naFila;
    int zona; //0 - Zona A | 1 - Zona B | 2 - Padaria 
    int vip; 
    int desistiu;
    int nPessoasAFrenteDesistir;
    int tempoChegada;
    int tempoMaxEsperaP;
    int tempoNaFila;
    int estado; // 0 = espera | 1 = coma
    int teste; //0 = nao fez teste | 1 - esta a levar soro | 2 = morreu | 3 = sobreviveu
};


struct Fila1 {
    int nPessoasEspera;
};

struct Fila2 {
    int nPessoasNormalEspera;
    int nPessoasPrioritariasEspera;
};

struct ZonaA
{
    int nPessoas;
};

struct ZonaB
{
    int nPessoas;
};

struct Padaria
{
    int nPessoas;
};





//simulador.c
int criaSocket();
int readConfiguracao();
void enviarMensagem(char *mensagemAEnviar);
int probabilidade (float valor);
int randomNumber(int max, int min);


//monitor.c
void socketservidor();
void leituraSocket(int sockfd);
void limpaFeedback();
void escreveFeedback();
void imprimeFeedback();
