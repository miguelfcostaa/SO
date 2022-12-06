//2079120 Miguel Costa
//2105319 Ines Jardim

#include "config.h"

int acabouSimulacao = FALSE;

int nDia = 0, nPessoasTotal = 0, nPessoasFila1 = 0, nPessoasFila2 = 0, nPessoasZonaA = 0, nPessoasFilaA = 0, nPessoasZonaB = 0,
 nPessoasFilaB = 0, nPessoasPadaria = 0, tempoMedio = 0;


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
    newsockfd = accept(sockfd, (struct sockaddr *)&serv_end, &cli_size);

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
        leituraSocket(newsockfd);
        printf("Monitor pronto. \n");
    }
    close(newsockfd);

}

/*
//funcao que recebe a informacao do servidor
void recebeMensagem(int newsockfd){
    int mensagem = 0;
    int estado = 0;
    int numJogadores = 0;
    int numTentativas = 0;
    int numDesistencias = 0;

    FILE* ficheiro = fopen("ftexto.txt", "a");                                                                           // a, cria ficheiro caso n�o exista

	if(ficheiro == NULL)
	{
		printf("Ficheiro nao foi criado");         								                                  // imprime na linha de comandos
	}
	else
	{
		while(estado!=20)
		{
			char linhaRecebe[MAXLINE+1];
			mensagem=recv(newsockfd,linhaRecebe,MAXLINE,0); 							                                        //linhaRcebe vai ter o conteudo do socket
			
			sscanf(linhaRecebe,"%d %d %d %d",&estado,&numJogadores,&numTentativas,&numDesistencias);
			
			switch(estado)
			{
				case 1: 
				{
				  fprintf(ficheiro, "\n JOGO SUDOKU \n" );	              //escreve no ficheiro
				  printf( "O JOGO IRA COMECAR DENTRO DE INSTANTES\n" );			                      //escreve no monitor
				  break;
				}
				case 2:
				{
			        fprintf(ficheiro, "Numero de jogadores em jogo: %d\n" , numJogadores);
			        printf( "Numero de Jogadores em jogo : %d\n" ,numJogadores); 
				  break;
				}
				case 3: 
				{
				    fprintf(ficheiro, "Numero de desistencias: %d\n" , numDesistencias);            
			        printf( "Numero de desistencias: %d\n" , numDesistencias);			     
				   break;
				} 
				case 4: 
				{
				    fprintf(ficheiro, "Numero total de tentativas : %d\n" ,numTentativas);            
			        printf( "Numero total de tentativas : %d\n" ,numTentativas);			     
				   break;
				}
			}
		}
	}
	if(estado==20){ 
	    fprintf(ficheiro, " ACABOU A SIMULACAO \n" );
	    printf( "ACABOU A SIMULACAO \n" );
	    fclose(ficheiro);
    }
}
*/


void leituraSocket(int sockfd) {
    int numero = 0;
    char buffer[MAXLINE];
    while (!acabouSimulacao) {
        numero = read(sockfd, buffer, MAXLINE); // Le a mensagem do socket e guarda no buffer
        if (numero == 0) {            // Quando chega ao fim
            printf("FIM");
            //break;
            return;
        } 
        else if (numero < 0) {
            printf("erro: nao foi possivel ler socket. \n");
        } 
        else {
            printf('Mensagem recebida. \n');
            printf(buffer);
        }
    }
}

void trataMensagem(char mensagem[]){
    printf(mensagem);
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
	printf("Pessoas na fila para a zona A: %d\n", nPessoasFilaA);
	printf("Pessoas na zona B: %d\n", nPessoasZonaB);
	printf("Pessoas na fila para a zona B: %d\n", nPessoasFilaB);
	printf("Pessoas na Padaria: %d\n", nPessoasPadaria);
	printf("Tempo medio de espera (minutos): %d\n", tempoMedio);	
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
                break;
            }
        }
    }
	escreveFeedback();
	//imprimeFeedback();
	return 0;
}
