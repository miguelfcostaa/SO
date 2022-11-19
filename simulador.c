//2079120 Miguel Costa
//2105319 Ines Jardim

#include "config.h"

int sockfd = 0; // inicia socket
int acabouSimulacao = FALSE;

int nDia = 0, nPessoasTotal = 0, nPessoasFila1 = 0, nPessoasFila2 = 0, nPessoasZonaA = 0, nPessoasFilaA = 0, nPessoasZonaB = 0,
 nPessoasFilaB = 0, nPessoasPadaria = 0, tempoMedio = 0;


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

//<<<<<<<<<<<<<<<<<<<<<<<< F.AUX >>>>>>>>>>>>>>>>>>>>>>>>>>>>>

int probabilidade (float valor) {
	return (rand() % 100) > (valor * 100);
}

int randomNumber(int max, int min) {
	return rand() % (max + 1 - min) + min;
}

//>>>>>>>>>>>>>>>>>>>>>> ESCRITA E IMPRESSAO >>>>>>>>>>>>>>>>>>>>>>>>>>>>

void limpaFeedback() {
	fclose(fopen("feedback.txt", "w"));
}

void escreveFeedback() {
	limpaFeedback();
	FILE *fp;
	fp = fopen("feedback.txt","a");

	if (fp == NULL){
		printf("Ocorreu um erro ao abrir o ficheiro\n");
	}
	else {
		if (!acabouSimulacao) {
            fprintf(fp, "%s", "Estado atual => Simulacao a decorrer!\n");
        } 
		else {
            fprintf(fp, "%s", "Estado atual => Simulacao acabou!\n");
        }
		fprintf(fp, "DIA %d\n", numeroDia);
		fprintf(fp, "\n");
		fprintf(fp, "Pessoas (total): %d\n", nPessoasTotal);
		fprintf(fp, "Pessoas a espera na fila 1: %d\n", nPessoasFila1);
		fprintf(fp, "Pessoas a espera na fila 2: %d\n", nPessoasFila2);
		fprintf(fp, "Pessoas na zona A: %d\n", nPessoasZonaA);
		fprintf(fp, "Pessoas na fila para a zona A: %d\n", nPessoasFilaA);
		fprintf(fp, "Pessoas na zona B: %d\n", nPessoasZonaB);
		fprintf(fp, "Pessoas na fila para a zona B: %d\n", nPessoasFilaB);
		fprintf(fp, "Pessoas na Padaria: %d\n", nPessoasPadaria);
		fprintf(fp, "Tempo medio de espera (minutos): %d\n", tempoMedio);
		fprintf(fp, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
		fclose(fp);
	}
}

void imprimeFeedback(){
	if (!acabouSimulacao) {
        fprintf(fp, "%s", "Estado atual => Simulacao a decorrer!\n");
    } 
	else {
    	fprintf(fp, "%s", "Estado atual => Simulacao acabou!\n");
    }
	printf(fp, "DIA %d\n", numeroDia);
	printf(fp, "\n");
	printf(fp, "Pessoas (total): %d\n", nPessoasTotal);
	printf(fp, "Pessoas a espera na fila 1: %d\n", nPessoasFila1);
	printf(fp, "Pessoas a espera na fila 2: %d\n", nPessoasFila2);
	printf(fp, "Pessoas na zona A: %d\n", nPessoasZonaA);
	printf(fp, "Pessoas na fila para a zona A: %d\n", nPessoasFilaA);
	printf(fp, "Pessoas na zona B: %d\n", nPessoasZonaB);
	printf(fp, "Pessoas na fila para a zona B: %d\n", nPessoasFilaB);
	printf(fp, "Pessoas na Padaria: %d\n", nPessoasPadaria);
	printf(fp, "Tempo medio de espera (minutos): %d\n", tempoMedio);	
	printf(fp, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");

}


//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

void main(){

	//lerficheiro();
	criaSocket();

}


