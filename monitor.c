#include "header.h"

int acabouSimulacao = FALSE;

int nDia = 0, nPessoasTotal = 0, nPessoasFila1 = 0, nPessoasFila2 = 0, nPessoasZonaA = 0, nPessoasZonaB = 0, nPessoasPadaria = 0, tempoMedio = 0;
nPessoasEmComa = 0; nPessoasComaMorreram = 0; nPessoasComaSobreviveram = 0;


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
                printf("Chegou uma pessoa \n");
                nPessoasTotal++;
                if (acontecimento == 1) {
                    nPessoasFila1++;
                } 
                else if (acontecimento == 2) {
                    nPessoasFila2++;
                }
                break;

            case 1: // uma pessoa desistiu
                printf("Uma pessoa desistiu \n");
                nPessoasTotal--;
                if (acontecimento == 1) {
                    nPessoasFila1--;
                } 
                else if (acontecimento == 2) {
                    nPessoasFila2--;
                }
                else if (acontecimento == 3) { //a pessoa cansou-se e foi embora
                    if (zona == ZONA_A){
                        nPessoasZonaA--;
                    }
                    else if (zona == ZONA_B){
                        nPessoasZonaB--;
                    }
                    else if (zona == PADARIA){
                        nPessoasPadaria--;
                    }
                    break;
                }
                break;

            case 2: // uma pessoa entrou na discoteca
                printf("Uma pessoa entrou na discoteca \n");
                if (acontecimento == 1) {
                    nPessoasFila1--;
                } 
                else if (acontecimento == 2) {
                    nPessoasFila2--;
                }
                if (zona == ZONA_A){
                    nPessoasZonaA++;
                }
                else if (zona == ZONA_B){
                    nPessoasZonaB++;
                }
                else if (zona == PADARIA){
                    nPessoasPadaria++;
                }
                break;

            case 3: //uma pessoa entrou em coma
                printf("Uma pessoa entrou em coma \n");
                nPessoasEmComa++;
                if (zona == ZONA_A){
                    nPessoasZonaA--;
                }
                else if (zona == ZONA_B){
                    nPessoasZonaB--;
                }
                else if (zona == PADARIA){
                    nPessoasPadaria--;
                }
                break;

            case 4: 
                printf("Uma pessoa entrou na discoteca sem esperar \n");
                nPessoasTotal++;
                if (zona == ZONA_A){
                    nPessoasZonaA++;
                }
                else if (zona == ZONA_B){
                    nPessoasZonaB++;
                }
                else if (zona == PADARIA){
                    nPessoasPadaria++;
                }
                break;
            case 5:
                printf("Uma pessoa sobriveu a coma \n");
                nPessoasEmComa--;
                nPessoasComaSobreviveram++;
                break;
            case 6:
                printf("Uma pessoa morreu \n");
                nPessoasEmComa--;
                nPessoasComaMorreram++;
                break;
            default:
                break;
        }
        imprimeFeedback();
            
    }
    if(estado == 99){ 
        printf("O tempo limite da simulacao foi atingido.\n");
        acabouSimulacao = TRUE;
    }
              
}



void leituraSocket(int sockfd) {
    int x = 0;
    char buffer[MAXLINE];
    while (!acabouSimulacao) {
        x = read(sockfd, buffer, MAXLINE); // Le a mensagem do socket e guarda no buffer
        if (x == 0) {            // Quando chega ao fim
            printf("FIM \n");
            break;
        } 
        else if (x < 0) {
            printf("erro: nao foi possivel ler socket. \n");
        } 
        else {
            printf('Mensagem recebida. \n');
            recebeInformacao(buffer);
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
        fprintf(fp, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
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
		fprintf(fp, "Pessoas na zona B: %d\n", nPessoasZonaB);
		fprintf(fp, "Pessoas na Padaria: %d\n", nPessoasPadaria);
		fprintf(fp, "Pessoas em coma: %d\n", nPessoasEmComa);
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
	printf("\t DIA %d\n", nDia);
	printf("Pessoas (total): %d\n", nPessoasTotal);
	printf("Pessoas a espera na fila 1: %d\n", nPessoasFila1);
	printf("Pessoas a espera na fila 2: %d\n", nPessoasFila2);
	printf("Pessoas na zona A: %d\n", nPessoasZonaA);
	printf("Pessoas na zona B: %d\n", nPessoasZonaB);
	printf("Pessoas na Padaria: %d\n", nPessoasPadaria);
	printf("Pessoas em coma: %d\n", nPessoasEmComa);
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
