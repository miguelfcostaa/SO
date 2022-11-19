//Grupo:
//2079120 Miguel Costa
//2105319 Ines Jardim

#include "config.h"

void escrever(sockfd){
	char String[100];
	FILE *fp;

	fp = fopen("feedback.txt","a");
	if (fp == NULL){
		printf("Ocorreu um erro ao abrir o ficheiro\n");
	}
	else {
		printf("Ficheiro carregado com sucesso\n");
	}
	
	strcpy(String,"Linha de teste");
	fputs(String, fp);
	fclose(fp);

}

void socketservidor(){
	int sockfd, newsockfd, clilen, childpid, servlen;
	struct sockaddr_un cli_addr, serv_addr;

	if ((sockfd = socket(AF_UNIX,SOCK_STREAM,0)) < 0)
		err_dump("server: can't open stream socket");

	bzero((char *)&serv_addr, sizeof(serv_addr));
	serv_addr.sun_family = AF_UNIX;
	strcpy(serv_addr.sun_path, UNIXSTR_PATH);

	servlen = strlen(serv_addr.sun_path) + sizeof(serv_addr.sun_family);
	unlink(UNIXSTR_PATH);

	if (bind(sockfd, (struct sockaddr *) &serv_addr, servlen) < 0)
		err_dump("server, can't bind local address");
	
	printf("Esperar Simulador \n");
	listen(sockfd, 5);

	
	clilen = sizeof(cli_addr);
	newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr,&clilen);
	if (newsockfd < 0)
		err_dump("server: accept error");
	
	if ((childpid = fork()) < 0)
		err_dump("server: fork error");
	
	else if (childpid == 0) {
		close(sockfd);
		escrever(newsockfd);
		exit(0);
		}
		close(newsockfd);
	}
}

int main(int argc, char *argv[]) {
	socketservidor();

	printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
	printf("1 - Ligar Discoteca               \n");
	printf("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");
	int opcao = 0;
	while (opcao != 1) {
        printf("Opcao: ");
        scanf("%d", &opcao); // Le valor introduzido
		if (opcao != 1){
			printf("Opcao invalida.");
			opcao = 0;
		}
	}
}
