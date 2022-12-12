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


#define UNIXSTR_PATH "/tmp/s.2079120"
#define MAXLINE 1024    //tamanho maximo do buffer
#define TRUE 1
#define FALSE 0

//Estado pessoa
#define DESISTIU 0
#define ESPERA 1


struct configuracao {
    int tempoMedioChegada;
    int tempoMedioEspera;
    int tempoMaxEspera;
    int tamanhoMaxFila1;
    int tamanhoMaxFila2;
    int tamanhoMaxZonaA;
    int tamanhoMaxZonaB;
    int tamanhoMaxPadaria;
    int probSerIdosa;
    int probSerVIP;
    int probDesistir;

};

struct pessoa {
    int id;
    int fila;
    int idoso;
    int desistiu;
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
