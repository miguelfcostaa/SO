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


//simulador.c
void lerficheiro();
int criaSocket();
int probabilidade (float valor);
int randomNumber(int max, int min);


//monitor.c
void socketservidor();
void leituraSocket(int sockfd);
void limpaFeedback();
void escreveFeedback();
void imprimeFeedback();
