
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


#define UNIXSTR_PATH "/tmp/s.unixstr"
#define UNIXDG_PATH  "/tmp/s.2079120"
#define UNIXDG_TMP   "/tmp/dgXXXXXXX"
#define MAXLINE 512
#define TRUE 1
#define FALSE 0


struct pessoa
{
    /* data */
    
};

//simulador.c
void lerficheiro();
int criaSocket();
void enviarMensagem(char *mensagemNova);
void main();

//monitor.c
void escrever(sockfd);
void socketservidor();
void leituraSocket(int sockfd);
void trataMensagem(char mensagem[]);
int main(int argc, char *argv[]);