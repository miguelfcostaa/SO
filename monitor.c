#include "header.h"

int acabouSimulacao = FALSE;

int nDia = 0, nPessoasTotal = 0, nPessoasFila1 = 0, nPessoasFila2 = 0, nPessoasJam = 0, nPessoasSmokingArea = 0, nPessoasPadaria = 0, tempoMedio = 0;
nPessoasEmComa = 0; nPessoasLevarSoro = 0; nPessoasComaMorreram = 0; nPessoasComaSobreviveram = 0; nPessoasAComer = 0; 
nPessoasAFumar = 0;


//<<<<<<<<<<<<<<<<<<<<<<< SOCKET >>>>>>>>>>>>>>>>>>>>>>>>>>>


void socketservidor() {

    int sockfd;  //criacao para a primeira comunicacao
    int newsockfd; //criacao para a segunda comunicacao
    int cli_size, server_size;
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
        printf("erro: nao foi possivel ligar o socket a um endereco. \n");
    }

    // Espera a conexao com o simulador
    printf("Começando a simulacao. Espera pelo simulador...\n");

    //servidor espera para aceitar 1 cliente para o socket stream
    listen(sockfd, 1);

    // Criacao de um novo scoket
    cli_size = sizeof(serv_end);
    newsockfd = accept (sockfd, (struct sockaddr *) &serv_end, &cli_size);

    if (newsockfd < 0) {        //verifica se houve erro na aceitacao da ligacao
        printf("erro: nao foi possivel aceitar a ligacao. \n");
    } 

    // Criacao de um novo processo
    int childpid;
    if ((childpid = fork()) < 0) {  // verifica a criacao do processo filho
        printf("erro: nao foi possivel criar o processo filho. \n"); 
    } 
    else if (childpid == 0) {       // Processo filho irá tratar das sucessivas leituras                                                  
        close(sockfd);              // fecha o socket do processo pai  
        printf("Monitor pronto. \n");                         
        recebeInformacao(newsockfd);
        
    }
    close(newsockfd);
    
}

void recebeInformacao(int newsockfd) {
    
    int mensagem = 0;
    int pessoaID = 0;
    int tempoMedido = 0;
    int estado = 0;
    int acontecimento = 0;
    int zona = 0; 

    //(pessoaID, timestamp, estado, acontecimento, zona)

    if ((pessoaID == 0) && (tempoMedido == 0) && (acontecimento == 0) && (zona == 0)) {
        if (estado == 0){
            printf("Bem vindo! A simulação comecou!\n");
        }
        
	}
    while (!acabouSimulacao) {

        char buffer[MAXLINE+1];
        mensagem = recv(newsockfd, buffer, MAXLINE, 0); 							                                        //linhaRcebe vai ter o conteudo do socket
                        
        sscanf(buffer,"%d %d %d %d %d", &pessoaID, &tempoMedido, &estado, &acontecimento, &zona);	  

        switch (estado)
        {
            case 0: //chegou uma pessoa
                printf("Chegou uma pessoa a fila com o id %d \n", pessoaID);
                nPessoasTotal++;
                if (acontecimento == 1) {
                    nPessoasFila1++;
                } 
                else if (acontecimento == 2) {
                    nPessoasFila2++;
                }
                break;

            case 1: // uma pessoa desistiu
                printf("Uma pessoa com id %d desistiu \n", pessoaID);
                nPessoasTotal--;
                if (acontecimento == 1) {
                    nPessoasFila1--;
                } 
                else if (acontecimento == 2) {
                    nPessoasFila2--;
                }
                else if (acontecimento == 3) { //a pessoa cansou-se e foi embora
                    if (zona == JAM){
                        nPessoasJam--;
                    }
                    else if (zona == SMOKING_AREA){
                        nPessoasSmokingArea--;
                    }
                    else if (zona == PADARIA){
                        nPessoasPadaria--;
                    }
                    break;
                }
                break;

            case 2: // uma pessoa entrou na discoteca
                printf("Uma pessoa com o id %d entrou na discoteca \n", pessoaID);
                if (acontecimento == 1) {
                    nPessoasFila1--;
                } 
                else if (acontecimento == 2) {
                    nPessoasFila2--;
                }
                if (zona == JAM){
                    nPessoasJam++;
                }
                else if (zona == SMOKING_AREA){
                    nPessoasSmokingArea++;
                }
                else if (zona == PADARIA){
                    nPessoasPadaria++;
                }
                break;

            case 3: //uma pessoa entrou em coma
                printf("Uma pessoa com o id %d entrou em coma \n", pessoaID);
                nPessoasEmComa++;
                if (zona == JAM){
                    nPessoasJam--;
                }
                else if (zona == SMOKING_AREA){
                    nPessoasSmokingArea--;
                }
                else if (zona == PADARIA){
                    nPessoasPadaria--;
                }
                break;

            case 4: 
                printf("Uma pessoa com o id %d entrou na discoteca sem esperar \n", pessoaID);
                nPessoasTotal++;
                if (zona == JAM){
                    nPessoasJam++;
                }
                else if (zona == SMOKING_AREA){
                    nPessoasSmokingArea++;
                }
                else if (zona == PADARIA){
                    nPessoasPadaria++;
                }
                break;
            case 5:
                printf("Uma pessoa com o id %d sobriveu a coma \n", pessoaID);
                nPessoasLevarSoro--;
                nPessoasEmComa--;
                nPessoasComaSobreviveram++;
                break;
            case 6:
                printf("Uma pessoa com o id %d morreu \n", pessoaID);
                nPessoasLevarSoro--;
                nPessoasEmComa--;
                nPessoasComaMorreram++;
                break;
            case 7:
                printf("Uma pessoa com o id %d mudou de zona \n", pessoaID);
                if (acontecimento == JAM){
                    nPessoasJam--;
                    if (zona == JAM){
                        nPessoasJam++;
                    }
                    else if (zona == SMOKING_AREA){
                        nPessoasSmokingArea++;
                    }
                    else if (zona == PADARIA){
                        nPessoasPadaria++;
                    }
                }
                if (acontecimento == SMOKING_AREA){
                    nPessoasSmokingArea--;
                    if (zona == JAM){
                        nPessoasJam++;
                    }
                    else if (zona == SMOKING_AREA){
                        nPessoasSmokingArea++;
                    }
                    else if (zona == PADARIA){
                        nPessoasPadaria++;
                    }
                }
                if (acontecimento == PADARIA){
                    nPessoasPadaria--;
                    if (zona == JAM){
                        nPessoasJam++;
                    }
                    else if (zona == SMOKING_AREA){
                        nPessoasSmokingArea++;
                    }
                    else if (zona == PADARIA){
                        nPessoasPadaria++;
                    }
                }
                break;
            case 8:
                printf("A pessoa com o id %d esta a levar soro \n", pessoaID);
                nPessoasLevarSoro++;
                break;
            case 9:
                printf("A pessoa com o id %d esta a comer \n", pessoaID);
                nPessoasAComer++;
                break;
            case 10:
                printf("A pessoa com o id %d acabou de comer \n", pessoaID);
                nPessoasAComer--;
                break;
            case 11:
                printf("A pessoa com o id %d foi fumar \n", pessoaID);
                nPessoasAFumar++;
                break;
            case 12:
                printf("A pessoa com o id %d acabou de fumar \n", pessoaID);
                nPessoasAFumar--;
                break;
            case 99:
                printf("O tempo limite da simulacao foi atingido.\n");
                acabouSimulacao = TRUE;
                break;
            default:
                break;
        }
        imprimeFeedback(); 
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
        fprintf(fp, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
		if (!acabouSimulacao) {
            fprintf(fp, "%s", "Estado atual => Simulacao a decorrer!\n");
        } 
		else {
            fprintf(fp, "%s", "Estado atual => Simulacao acabou!\n");
        }
		fprintf(fp, "\t \n");
		fprintf(fp, "Pessoas (total): %d\n", nPessoasTotal);
		fprintf(fp, "Pessoas a espera na fila 1: %d\n", nPessoasFila1);
		fprintf(fp, "Pessoas a espera na fila 2: %d\n", nPessoasFila2);
		fprintf(fp, "Pessoas no JAM: %d\n", nPessoasJam);
		fprintf(fp, "Pessoas na area de fumar: %d\n", nPessoasSmokingArea);
        fprintf(fp, "Pessoas a fumar: %d\n", nPessoasAFumar);
		fprintf(fp, "Pessoas na Padaria: %d\n", nPessoasPadaria);
        fprintf(fp, "Pessoas a comer: %d\n", nPessoasAComer);
		fprintf(fp, "Pessoas em coma: %d\n", nPessoasEmComa);
        fprintf(fp, "Pessoas a levar soro (max 2): %d\n", nPessoasLevarSoro);
        fprintf(fp, "Pessoas que morreram de coma: %d\n", nPessoasComaMorreram);
        fprintf(fp, "Pessoas que sobreviveram de coma: %d\n", nPessoasComaSobreviveram);
		fprintf(fp, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
		fclose(fp);
	}
}

void imprimeFeedback(){
    printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	if (!acabouSimulacao) {
        printf("%s", "Estado atual => Simulacao a decorrer!\n");
    } 
	else {
    	printf("%s", "Estado atual => Simulacao acabou!\n");
    }
	printf("\t \n");
	printf("Pessoas (total): %d\n", nPessoasTotal);
	printf("Pessoas a espera na fila 1: %d\n", nPessoasFila1);
	printf("Pessoas a espera na fila 2: %d\n", nPessoasFila2);
	printf("Pessoas no JAM: %d\n", nPessoasJam);
	printf("Pessoas na area de fumar: %d\n", nPessoasSmokingArea);
    printf("Pessoas a fumar: %d\n", nPessoasAFumar);
	printf("Pessoas na Padaria: %d\n", nPessoasPadaria);
    printf("Pessoas a comer: %d\n", nPessoasAComer);
	printf("Pessoas em coma: %d\n", nPessoasEmComa);
    printf("Pessoas a levar soro (max 2): %d\n", nPessoasLevarSoro);
    printf("Pessoas que morreram de coma: %d\n", nPessoasComaMorreram);
    printf("Pessoas que sobreviveram de coma: %d\n", nPessoasComaSobreviveram);	
	printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	escreveFeedback();
}

//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< MAIN >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


int main() {

	printf("~~~~~~~~~~~~~~~~~~~~~~\n");
	printf("1 - Começar simulacao \n");
    printf("2 - Fechar            \n");
	printf("~~~~~~~~~~~~~~~~~~~~~~\n");
	int opcao = 0;
    while (!acabouSimulacao){
        while (opcao != 1){
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
            case 2:
                printf("A fechar simulacao...\n");
                return 0;
                break;
            default:
                printf("erro: opcao invalida! \n");
                break;
            }
        }
    }
	escreveFeedback();
	return 0;
}
