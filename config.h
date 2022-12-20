#include <stdio.h>
#include <ctype.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>


#define UNIXSTR_PATH "/tmp/s.2079120"
#define MAXLINE 1024    //tamanho maximo do buffer
#define SIZE_TASKS 100000
#define TRUE 1
#define FALSE 0


//Estado pessoa
#define ESPERA 0
#define COMA 1
#define MULHER 2
#define HOMEM 3


//Resultado do teste
#define NAO_FEZ_TESTE 0
#define MORREU 1
#define SOBREVIVEU 2

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
    int tempoFazerTeste;
    float probSerVIP;
    float probDesistir;
    float probSerMulher;
    float probSerHomem;
    float probMorrerComa;
};

struct pessoa {
    int id;
    int sexualidade; //MULHER - 0 | HOMEM - 1
    int fila;
    int zona; //0 - Zona A | 1 - Zona B | 2 - Padaria 
    int vip; 
    int desistiu;
    int nPessoasAFrenteDesistir;
    int estado; // 0 = espera | 1 = coma
    int resultadoTeste; //0 = nao fez teste | 1 = morreu | 2 = sobreviveu
    int tempoMaxEsperaP;
};


struct Fila1 {
    int nPessoasEspera;
    sem_t filaEspera;
    int nFilasDisponiveis;
};

struct Fila2 {
    int nPessoasNormalEspera;
    int nPessoasPrioritariasEspera;
    sem_t filaEsperaPrioritaria;
    sem_t filaEsperaNormal;
    int nFilasDisponiveis;
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
