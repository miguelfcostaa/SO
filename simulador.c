//2079120 Miguel Costa
//2105319 Ines Jardim

#include "config.h"

int sockfd = 0; // inicia socket

//Variaveis do ficheiro de simulacao
int tempoMedioChegada = 0;
int tempoMedioEspera = 0;
int tamanhoMaxFila1 = 0;
int tamanhoMaxFila2 = 0;
int tamanhoMaxZonaA = 0;
int tamanhoMaxZonaB = 0;
int tamanhoMaxPadaria = 0;
int probSerIdosa = 0;
int probSerVIP = 0;
int probDesistir = 0;


int criaSocket() {

    int sockfd, newsockfd, clilen, childpid, server_size;
	struct sockaddr_un cli_addr, serv_end;

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
    int x = 0;
    while (connect(sockfd, (struct sockaddr *)&serv_end, server_size) < 0) {
        if (x == 0) {
            printf("Espera pelo monitor...\n");
            x = 1;
        }
    }
    printf("Simulador pronto. \n");
    //enviarMensagem("Simulador: Hello!");
    return sockfd;
    
}


int readConfiguracao(){     //funcao para ler a configuracao

    FILE* fp = fopen("simulador.conf", "r");    //abre o ficheiro para ler
                                                          
    if(fp != NULL){
       char linha[50];               //guarda a linha                                   
       int valor;                    //guarda o valor do parametro                                             
       char parametro[50];           //guarda o parametro

       while(fgets(linha, sizeof(linha), fp) != NULL) {                      // caso o ficheiro não esteja vazio guarda a linha do ficheiro na variável line                      
            
            sscanf(linha, "%s : %d", parametro , &valor);                            // faz scan da linha
               
            if(strcmp(parametro, "tempoMedioChegada") == 0)
            {                
                tempoMedioChegada = valor;
            }
            if(strcmp(parametro, "tempoMedioEspera") == 0)
            {                            
                tempoMedioEspera = valor;
            }  
            if(strcmp(parametro, "tamanhoMaxFila1") == 0)
            {                            
                tamanhoMaxFila1 = valor;
            } 
            if(strcmp(parametro, "tamanhoMaxFila2") == 0)
            {                            
                tamanhoMaxFila2 = valor;
            }   
            if(strcmp(parametro, "tamanhoMaxZonaA") == 0)
            {
                tamanhoMaxZonaA = valor;
            }
            if(strcmp(parametro, "tamanhoMaxZonaB") == 0)
            {
                tamanhoMaxZonaB = valor;
            }
            if(strcmp(parametro, "tamanhoMaxPadaria") == 0)
            {
                tamanhoMaxPadaria = valor;
            }
            if(strcmp(parametro, "probSerIdosa") == 0)
            {
                probSerIdosa = valor;
            }
            if(strcmp(parametro, "probSerVIP") == 0)
            {					     
                probSerVIP = valor;
            }
            if(strcmp(parametro, "probDesistir") == 0)					     
            {
                probDesistir = valor;
            }
        }
        printf( "Tempo Medio Chegada: %d\n" ,tempoMedioChegada);	
        //printf('%i\n', tempoMedioEspera, '%i\n', tamanhoMaxFila1, '%i\n', tamanhoMaxFila2, '%i\n', tamanhoMaxZonaA, '%i\n',tamanhoMaxZonaB,'%i\n', tamanhoMaxPadaria,'%i\n',  probSerIdosa, '%i\n', probSerVIP, '%i\n', probDesistir);
        fclose(fp);
    }
    else {
        printf("erro: nao foi possivel ler o ficheiro de configuracao. \n");
    }
}

void enviarMensagem(char *mensagemAEnviar) {        // funcao que envia mensagens para o monitor
   int numero;
    char mensagem[MAXLINE];
    if (strcpy(mensagem, mensagemAEnviar) != 0) {
        numero = strlen(mensagem) + 1;
        if (write(sockfd, mensagem, numero) != numero) {
            printf("Erro no write!\n");
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

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
/*
void simulacao(int sockfd){
    InicializaValores();
    dadosMonitor(sockfd,1,0,0,0);
    int um = 1;
    int dois = 2;
    //pthread_create(&tarefasEquipas[0],NULL,equipaTarefas,&um);
    //pthread_create(&tarefasEquipas[1],NULL,equipaTarefas,&dois);

    printSudoku(1);
    int i = 0;
    for(int i = 0; i<numMaxJogadores;i++){
        pthread_create(&tarefasJogadores[i],NULL,jogador,NULL);
    }
    for(int i = 0; i<numMaxJogadores;i++){
        pthread_join(tarefasJogadores[i],NULL);
    }
    int numTentativasEquipa1 = numTentativasJogadoresEquipa1;
    int numTentativasEquipa2 = numTentativasJogadoresEquipa2;
    printf("A equipa 1 terminou o jogo com: %d \n",numTentativasEquipa1);
    printf("A equipa 2 terminou o jogo com: %d \n",numTentativasEquipa2);
    printf("//////////////////////\n TABULEIRO SOLUCAO \n /////////////////\n");
    printSudoku(2);
    if(numTentativasErradasJogadoresEquipa1>numTentativasErradasJogadoresEquipa2){
        printf("///////////////////////////\nA EQUIPA VENCEDORA É A EQUIPA 2\n////////////////////\n");
    }else if(numTentativasErradasJogadoresEquipa1<numTentativasErradasJogadoresEquipa2){
         printf("///////////////////////////\nA EQUIPA VENCEDORA É A EQUIPA 1\n////////////////////\n");
    }else{
         printf("///////////////////////////\nAS EQUIPAS EMPATARAM\n////////////////////\n");
    }
    //pthread_join(tarefasEquipas[0],NULL);
    //pthread_join(tarefasEquipas[1],NULL);

    dadosMonitor(sockfd,20,0,0,0);
}
*/

int main(int argc, char const * argv[]){


    criaSocket();
    readConfiguracao();
    

    /*sockfd = criarSocket();
    simulacao(sockfd);
    close(sockfd);*/
    return 0;



}


