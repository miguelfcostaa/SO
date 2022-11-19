//Participantes do grupo:
//2079120 Miguel Costa
//2105319 Ines Jardim

#include "config.h"

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

void str_cliente(int sockfd){
	int n;
	FILE* fp;
	char sendline[MAXLINE], recvline[MAXLINE+1];

	fp = fopen("feedback.txt","r");

	while (fgets(sendline, MAXLINE, fp) != NULL) {

		n = strlen(sendline);
		if (writen(sockfd, sendline, n) != n)
			printf("str_cli: writen error on socket");

		/* Tenta ler string de sockfd. Note-se que tem de 
		   terminar a string com \0 */

		n = readline(sockfd, recvline, MAXLINE);
		if (n<0)
			printf("str_cli:readline error");
		recvline[n] = 0;

		fputs(recvline, stdout);
	}
	if (ferror(fp))
		printf("str_cli: error reading file");
}

void socketcliente(){
	struct sockaddr_un serv_addr;
	int servlen;

	if ((sockfd= socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
		printf("client: can't open stream socket");

	bzero((char *) &serv_addr, sizeof(serv_addr));
	
	serv_addr.sun_family = AF_UNIX;
	strcpy(serv_addr.sun_path, UNIXSTR_PATH);
	servlen = strlen(serv_addr.sun_path) + sizeof(serv_addr.sun_family);

	if (connect(sockfd, (struct sockaddr *) &serv_addr, servlen) < 0)
		printf("client: can't connect to server");

	str_cliente(sockfd);

	close(sockfd);
	exit(0);
}



void main(){

	lerficheiro();
	socketcliente();

}


