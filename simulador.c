//2079120 Miguel Costa
//2105319 Ines Jardim

#include "config.h"

int sockfd = 0; // inicia socket
int pessoaID = 0;
struct configuracao config;
struct Fila1 filas1;
struct Fila2 filas2;


//Trincos e Semaforos
pthread_mutex_t mutexCriarPessoa;
pthread_mutex_t mutexFilaEspera;
pthread_mutex_t mutexVariaveisSimulacao;
pthread_mutex_t mutexVariaveisHospital;
sem_t semaforoEnviarMensagem;
sem_t semaforoMedicos;
sem_t semaforoDoentes;

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
        printf( "Tempo Medio Chegada: %d; Configuracao lida com sucesso.\n" ,tempoMedioChegada);	
        fclose(fp);
    }
    else {
        printf("erro: nao foi possivel ler o ficheiro de configuracao. \n");
    }
}


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

void dadosMonitor(int sockfd, int estado, int numJogadores, int numTentativas, int numDesistencias){
    sem_wait(&semDadosMonitor);
    char buffer[MAXLINE];
    int lenInformacao = 0;
    sprintf(buffer,"%d %d %d %d",estado,numJogadores,numTentativas,numDesistencias);
    lenInformacao=strlen(buffer)+1;
    if(send(sockfd,buffer,lenInformacao,0)!=lenInformacao){
    perror("Erro ao enviar os dados\n");   
    }
    sleep(1);
    sem_post(&semDadosMonitor);
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

struct pessoa criaPessoa() {

    pthread_mutex_lock(&mutexCriarPessoa);

    struct pessoa people;

    people.id = pessoaID;
    people.fila = randomNumber(2, 1);
    people.idoso = probabilidade(config.probSerIdosa);
    people.desistiu = FALSE;
    if (people.idoso) {
        people.fila = 2;
    }
    if (people.fila == 1) {
        people.nPessoasAFrenteDesistir = randomNumber(config.tamanhoMaxFila1, (config.tamanhoMaxFila1 * 2) / 3);
    } 
    else if (people.fila == 2) {
        people.nPessoasAFrenteDesistir = randomNumber(config.tamanhoMaxFila2, (config.tamanhoMaxFila2 * 2) / 3);
    }
    people.estado = ESPERA;
    people.tempoMaxEsperaP = randomNumber(config.tempoMaxEspera, (config.tempoMaxEspera * 2) / 3);

    printf("Criado Pessoa %d: \n", pessoaID);
    pessoaID++;
    pthread_mutex_unlock(&mutexCriarPessoa);
    return people;
}

void FilaEspera(struct pessoa *people) {

    pthread_mutex_lock(&mutexFilaEspera);
    char mensagem[MAXLINE];
    int tempoEspera;
    int valorSemaforo = -1;

    if (people->fila == 1) { // CENTRO TESTES 1
        if (filas1.nPessoasEspera < config.tamanhoMaxFila1) {
            printf("A pessoa com o id %d chegou a fila 1.\n", people->id);
            sprintf(mensagem, "%d-%d-%d-%d", idPessoa, timestamp, 0, 1);
            enviarMensagem(mensagem);
            if (people->numeroPessoasAFrenteParaDesistir < filas1.nPessoasEspera ) {
                printf("A pessoa com o id %d desistiu da fila 1 porque tinha muita gente a frente.\n", people->id);
                pthread_mutex_unlock(&mutexFilaEspera);
                //sprintf(mensagem, "%d-%d-%d-%d", people->id, timestamp, 2, 1);
                enviarMensagem(mensagem);
                people->desistiu = TRUE;
                return;
            }
            people->tempoChegadaFilaEspera = timestamp;
            filas1.nPessoasEspera++;
            if (filas1.nPessoasEspera > config.tamanhoMaxFila1 - 5) {
                people->tipoTeste = TESTE_RAPIDO;
            }
            pthread_mutex_unlock(&mutexFilaEspera);
            // printf("A pessoa com o id %d chegou1 a fila.\n", pessoa->id);
            sem_wait(&centroTestes1.filaEspera);
            pthread_mutex_lock(&mutexVariaveisSimulacao);
            tempoEspera = minutosDecorridos - people->tempoChegadaFilaEspera;
            if (tempoEspera > people->tempoMaxEsperaP) { // passou muito tempo à
                // espera, a pessoa desiste
                people->desistiu = TRUE;
                sem_getvalue(&filas1.filaEspera, &valorSemaforo);
                if (valorSemaforo < filas1.numeroPostosDisponiveis) {
                sem_post(&filas1.filaEspera);
                }
                printf(VERMELHO "A pessoa com o id %d desistiu no centro 1 porque passou muito tempo a espera.\n" RESET, people->id);
                pthread_mutex_unlock(&mutexVariaveisSimulacao);
                sprintf(mensagem, "%d-%d-%d-%d", people->id, tempoEspera, 2, 1);
                enviarMensagem(mensagem);
                pthread_mutex_lock(&mutexVariaveisSimulacao);
            } 
            else { // não desiste, vai ser testada
                pthread_mutex_unlock(&mutexVariaveisSimulacao);
                // printf("A pessoa com o id %d vai ser testada quando houver ponto
                // livre no centro 1.\n", pessoa->id);
                // sem_wait(&centroTestes1.pontosTestagem);
                printf(VERDE "A pessoa com o id %d foi testada no centro 1.\n" RESET,
                    people->id);
                sprintf(mensagem, "%d-%d-%d-%d", people->id, tempoEspera, 1, 1);
                enviarMensagem(mensagem);
                pthread_mutex_lock(&mutexVariaveisSimulacao);
                for (int i = 0; i < configuracao.numeroPontosTestagemCentro1; i++) {
                    // printf("%d\n", *(tempoCooldownPontosTestagemCentro1 + index));
                    if (tempoCooldownPontosTestagemCentro1[index] == -1) // ponto testagem está livre, começa-se o cooldown
                    {
                        centroTestes1.numeroPostosDisponiveis--;
                        tempoCooldownPontosTestagemCentro1[index] =
                            configuracao.tempoCooldownPontosTestagem;
                        // printf("%d\n", *(tempoCooldownPontosTestagemCentro1 + index));
                        break;
                    }
                }
            }
            pthread_mutex_unlock(&mutexVariaveisSimulacao);
            pthread_mutex_lock(&mutexFilaEspera);
            // sem_post(&centroTestes1.filaEspera);
            filas1.numeroPessoasEspera--;
            pthread_mutex_unlock(&mutexFilaEspera);
        }
    }
}


int main(int argc, char const * argv[]){

    criaSocket();
    readConfiguracao();
    
    /*sockfd = criarSocket();
    simulacao(sockfd);
    close(sockfd);*/
    return 0;
}


