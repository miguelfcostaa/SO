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

void socketsimulador(){
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

	listen(sockfd, 5);

	for (;;) {
		clilen = sizeof(cli_addr);
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr,&clilen);
		if (newsockfd < 0)
			err_dump("server: accept error");
		if ((childpid = fork()) < 0)
			err_dump("server: fork error");
		else if (childpid == 0) {
			close(sockfd);
			str_echo(newsockfd);
			exit(0);
		}
		close(newsockfd);
	}
}

void main(){

	lerficheiro();
	socketsimulador();

}


