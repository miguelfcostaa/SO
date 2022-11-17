//Grupo:
//2079120 Miguel Costa
//2105319 Ines Jardim

#include "config.h"
#include "unix.h"

void escrever(){
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

void socketmonitor(){
	int sockfd, servlen;
	struct sockaddr_un serv_addr;

	if ((sockfd= socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
		err_dump("client: can't open stream socket");

	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sun_family = AF_UNIX;
	strcpy(serv_addr.sun_path, UNIXSTR_PATH);
	servlen = strlen(serv_addr.sun_path) + sizeof(serv_addr.sun_family);

	if (connect(sockfd, (struct sockaddr *) &serv_addr, servlen) < 0)
		err_dump("client: can't connect to server");

	str_cli(stdin, sockfd);

	close(sockfd);
	exit(0);
}

int main(int argc, char *argv[]) {
	escrever();
	socketmonitor();

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
