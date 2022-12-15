#include <ctype.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
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
#define TRUE 1
#define FALSE 0

//Estado pessoa
#define DESISTIU 0
#define ESPERA 1
#define ZONA_A 0
#define ZONA_B 1
#define MULHER 0
#define HOMEM 1


struct configuracao {
    int tempoMedioChegada;
    int tempoMedioEspera;
    int tempoMaxEspera;
    int tamanhoMaxFila1;
    int tamanhoMaxFila2;
    int tamanhoMaxZonaA;
    int tamanhoMaxZonaB;
    int tamanhoMaxPadaria;
    int probSerVIP;
    int probDesistir;
    int probSerMulher;
    int probSerHomem;
};

struct pessoa {
    int id;
    int sexualidade; //MULHER - 0 | HOMEM - 1
    int fila;
    int zona; //0 - Zona A | 1 - Zona B | 2 - Padaria
    bool idoso; 
    bool vip; 
    bool desistiu;
    int nPessoasAFrenteDesistir;
    int estado; // 0 = espera | 
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

struct Padaria {
    int nPessoasEspera;
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
