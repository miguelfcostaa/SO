//2079120 Miguel Costa
//2105319 Ines Jardim

#include "config.h"

int acabouSimulacao = FALSE;

int nDia = 0, nPessoasTotal = 0, nPessoasFila1 = 0, nPessoasFila2 = 0, nPessoasZonaA = 0, nPessoasFilaA = 0, nPessoasZonaB = 0,
 nPessoasFilaB = 0, nPessoasPadaria = 0, tempoMedio = 0;


void socketservidor() {
    // sockfd -> criacao para a primeira comunicacao
    // novoSocket -> criacao para a segunda comunicacao
    int sockfd, novoSocket, tamanhoCliente, server_size;
    struct sockaddr_un serv_end, serv_addr;

    // Verifica a criacao do socket
    if ((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
        printf("Erro ao criar o Socket\n");
    }

    // Incializa os valores do buffer a zero
    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sun_family = AF_UNIX;
    strcpy(serv_addr.sun_path, UNIXSTR_PATH);
    server_size = strlen(serv_addr.sun_path) + sizeof(serv_addr.sun_family);
    unlink(UNIXSTR_PATH);

    // Liga o socket a um endereco
    if (bind(sockfd, (struct sockaddr *)&serv_addr, server_size) < 0) {
        printf("Erro a ligar o socket a um endereco\n");
    }

    // Espera a conexao com o simulador
    printf("Esperando pelo simulador...\n");
    listen(sockfd, 1);

    // Criacao de um novo scoket
    tamanhoCliente = sizeof(serv_end);
    novoSocket = accept(sockfd, (struct sockaddr *)&serv_end, &tamanhoCliente);
    if (novoSocket < 0) {
        printf("Erro na aceitacao \n");
    } // Verifica o erro na aceitacao da ligacao

    // Criacao de um novo processo
    int pid;
    if ((pid = fork()) < 0) {
        printf("Erro ao criar o processo filho \n"); // Erro na criacao do processo
                                                     // filho
    } else if (pid == 0) {                           // Processo filho irá tratar das sucessivas leituras e
                                                     // fecha o socket do processo pai
        close(sockfd);
        leituraSocket(novoSocket);
    }
    close(novoSocket);
}

void leituraSocket(int sockfd) {
    int numero = 0;
    char buffer[MAXLINE];
    while (!acabouSimulacao) {
        numero = read(sockfd, buffer, MAXLINE); // Le a mensagem do socket e guarda no buffer
        if (numero == 0) {            // Quando chega ao fim
            //printf("FIM");
            break;
        } 
		else if (numero < 0) {
            printf("erro: nao foi possivel ler o socket. \n");
        } 
		else {
            printf("Mensagem Recebida");
            //trataMensagem(buffer);
        }
    }
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
		fprintf(fp, "\t DIA %d\n", nDia);
		fprintf(fp, "Pessoas (total): %d\n", nPessoasTotal);
		fprintf(fp, "Pessoas a espera na fila 1: %d\n", nPessoasFila1);
		fprintf(fp, "Pessoas a espera na fila 2: %d\n", nPessoasFila2);
		fprintf(fp, "Pessoas na zona A: %d\n", nPessoasZonaA);
		fprintf(fp, "Pessoas na fila para a zona A: %d\n", nPessoasFilaA);
		fprintf(fp, "Pessoas na zona B: %d\n", nPessoasZonaB);
		fprintf(fp, "Pessoas na fila para a zona B: %d\n", nPessoasFilaB);
		fprintf(fp, "Pessoas na Padaria: %d\n", nPessoasPadaria);
		fprintf(fp, "Tempo medio de espera (minutos): %d\n", tempoMedio);
		fprintf(fp, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
		fclose(fp);
	}
}

void imprimeFeedback(){
	if (!acabouSimulacao) {
        printf("%s", "Estado atual => Simulacao a decorrer!\n");
    } 
	else {
    	printf("%s", "Estado atual => Simulacao acabou!\n");
    }
	printf("\t DIA %d\n", nDia);
	printf("Pessoas (total): %d\n", nPessoasTotal);
	printf("Pessoas a espera na fila 1: %d\n", nPessoasFila1);
	printf("Pessoas a espera na fila 2: %d\n", nPessoasFila2);
	printf("Pessoas na zona A: %d\n", nPessoasZonaA);
	printf("Pessoas na fila para a zona A: %d\n", nPessoasFilaA);
	printf("Pessoas na zona B: %d\n", nPessoasZonaB);
	printf("Pessoas na fila para a zona B: %d\n", nPessoasFilaB);
	printf("Pessoas na Padaria: %d\n", nPessoasPadaria);
	printf("Tempo medio de espera (minutos): %d\n", tempoMedio);	
	printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	escreveFeedback();
}




int main(int argc, char *argv[]) {

	printf("~~~~~~~~~~~~~~~~~~~~~~\n");
	printf("1 - Começar simulacao \n");
	printf("~~~~~~~~~~~~~~~~~~~~~~\n");
	int opcao = 0;
	while (opcao != 1) {
        printf("Escolha uma opcao: ");
        scanf("%d", &opcao); // Le valor introduzido
		switch (opcao)
		{
		case 0:
			printf("erro: opcao invalida! \n");
			break;
		case 1:
			socketservidor();
			break;
		default:
			break;
		}
	}
	//escreveFeedback();
	imprimeFeedback();
	return 0;
}
