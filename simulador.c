//2079120 Miguel Costa
//2105319 Ines Jardim

#include "config.h"

int sockfd = 0; // inicia socket


int criaSocket() {

    int sockfd, newsockfd, clilen, childpid, server_size;
	struct sockaddr_un cli_addr, serv_end;

	// Cria o socket
    sockfd = socket(AF_UNIX, SOCK_STREAM, 0); 
    if (sockfd < 0) {
        printf("erro: nao foi possivel criar o socket. \n");
    }

    // Zerar o socket
    bzero((char *)&serv_end, sizeof(serv_end));


    // Familia do socket
    serv_end.sun_family = AF_UNIX;
    strcpy(serv_end.sun_path, UNIXSTR_PATH);
    
    server_size = strlen(serv_end.sun_path) + sizeof(serv_end.sun_family);

    // Estabelecer a ligacao com o socket
    int x = 0;
    while (connect(sockfd, (struct sockaddr *)&serv_end, server_size) < 0) {
        if (x == 0) {
            printf("Espera pelo monitor...\n");
            x = 1;
        }
    }
    printf("Simulador pronto. \n");
    enviarMensagem("Simulador: Hello!");
    return sockfd;
    
}

void enviarMensagem(char *mensagemAEnviar) {// funcao que envia mensagens para o monitor
   int numero;
    char mensagem[MAXLINE];
    if (strcpy(mensagem, mensagemAEnviar) != 0) {
        numero = strlen(mensagem) + 1;
        if (write(sockfd, mensagem, numero) != numero) {
            printf("Erro no write!\n");
        }
    }
}


//<<<<<<<<<<<<<<<<<<<<<<<< F.AUX >>>>>>>>>>>>>>>>>>>>>>>>>>>>>

int probabilidade (float valor) {
	return (rand() % 100) > (valor * 100);
}

int randomNumber(int max, int min) {
	return rand() % (max + 1 - min) + min;
}



//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

void main(){

	criaSocket();

}


