//2079120 Miguel Costa
//2105319 Ines Jardim

#include "config.h"

int sockfd = 0; // inicia socket

void lerficheiro(){
	FILE *fp;
	char Linha[100];

	fp = fopen("feedback.txt","r");
   	
	if (fp == NULL){
 		printf("Problemas na abertura do ficheiro\n");		
  	}
	
	while (!feof(fp)){
		fgets(Linha, 100, fp);
		printf("Linha: %s", Linha);
	}
	fclose(fp);
}

int criaSocket() {
    struct sockaddr_un serv_end;
    int server_size;

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
    int varprint = 0;
    while (connect(sockfd, (struct sockaddr *)&serv_end, server_size) < 0) {
        if (varprint == 0) {
            printf("Espera pelo monitor...\n");
            varprint = 1;
        }
    }
    printf("Simulador pronto. Esperando pelo início da simulação...\n");
    return sockfd;
}

// Envia mensagens para o monitor
void enviarMensagem(char *mensagemNova) {
    int numero;
    char mensagem[MAXLINE];
    if (strcpy(mensagem, mensagemNova) != 0) {
        numero = strlen(mensagem) + 1;
        if (write(sockfd, mensagem, numero) != numero) {
            printf("erro: nao foi possivel escrever. \n");
        }
    }
}




void main(){

	lerficheiro();
	criaSocket();

}


