#include "config.h"

//Variaveis globais
int sockfd = 0; // inicia socket
int pessoaID = 0;
int tempoDecorrido = 0; // tempo decorrido em segundos

//configuracao e filas
struct configuracao config;
struct Fila1 filas1;
struct Fila2 filas2;


//Trincos e Semaforos
pthread_mutex_t mutexCriarPessoa;
pthread_mutex_t mutexFilaDeEspera;
pthread_mutex_t mutexVariaveisSimulacao;
sem_t semaforoEnviarMensagem;
sem_t semaforoMedicos;
sem_t semaforoDoentes;


//Variaveis do ficheiro de simulacao
int tamanhoDiscoteca = 0;
int tempoLimiteSimulacao = 0;
int tamanhoMaxFila1 = 0;
int tamanhoMaxFila2 = 0;
int tamanhoMaxZonaA = 0;
int tamanhoMaxZonaB = 0;
int tamanhoMaxPadaria = 0;
int tempoMedioChegada = 0;
int tempoMedioEspera = 0;
int tempoMaxEspera = 0;
int tempoFazerTeste = 0;
int probSerVIP = 0;
int probDesistir = 0;
int probSerMulher = 0;
int probSerHomem = 0;
int probMorrerComa = 0;

//TAREFAS
pthread_t tasksID[SIZE_TASKS]; // pessoas e médicos
struct pessoa *pessoasCriadas[100000];


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
    people.sexualidade = randomNumber(HOMEM, MULHER);
    people.fila = randomNumber(2, 1);
    people.zona = randomNumber(2, 0);
    people.idoso = FALSE;
    people.vip = probabilidade(config.probSerVIP);
    people.desistiu = FALSE;


    if (people.vip == 1) {
        people.fila = 2;
    }
    if (people.fila == 1) {
        people.nPessoasAFrenteDesistir = randomNumber(config.tamanhoMaxFila1, (config.tamanhoMaxFila1 * 2) / 3);
    } 
    else if (people.fila == 2) {
        people.nPessoasAFrenteDesistir = randomNumber(config.tamanhoMaxFila2, (config.tamanhoMaxFila2 * 2) / 3);
    }
    if (people.zona == ZONA_A) { //ZONA A
        people.nPessoasAFrenteDesistir = randomNumber((config.tamanhoMaxZonaA * 2), config.tamanhoMaxZonaA);
    }
    if (people.zona == ZONA_B) { //ZONA B
        people.nPessoasAFrenteDesistir = randomNumber((config.tamanhoMaxZonaB * 2), config.tamanhoMaxZonaB);
    }
    if (people.zona == PADARIA) { //ZONA DA PADARIA
        people.nPessoasAFrenteDesistir = randomNumber((config.tamanhoMaxPadaria * 2), config.tamanhoMaxPadaria);
    }
    people.estado = ESPERA;
    people.resultadoTeste = NAO_FEZ_TESTE;
    people.tempoMaxEsperaP = randomNumber(config.tempoMaxEspera, (config.tempoMaxEspera * 2) / 3);

    printf("Criado Pessoa %d: \n", pessoaID);
    /*
    printf("id: %d\n",people.id);
    printf("sexualidade: %d\n",people.sexualidade);
    printf("fila: %d\n",people.fila);
    printf("zona: %d\n",people.zona);
    printf("idoso: %d\n",people.idoso);
    printf("vip: %d\n",people.vip);
    printf("desistiu: %d\n",people.desistiu);
    printf("pessoas a frente: %d\n",people.nPessoasAFrenteDesistir);
    printf("estado: %d\n",people.estado);
    printf("tempo: %d\n",people.tempoMaxEsperaP);*/
    pessoaID++;
    pthread_mutex_unlock(&mutexCriarPessoa);
    return people;
}



void FilaDeEspera(struct pessoa *people) {

    pthread_mutex_lock(&mutexFilaDeEspera);
    char mensagem[MAXLINE];
    int tempoEspera;
    int valorSemaforo = -1;

    if (people->fila == 1) { // FILA 1
        if (filas1.nPessoasEspera < config.tamanhoMaxFila1) {
            printf("A pessoa com o id %d chegou a fila 1.\n", people->id);
            //sprintf(mensagem, "%d-%d-%d-%d", idPessoa, timestamp, 0, 1);
            //enviarMensagem(mensagem);
            if (people->nPessoasAFrenteDesistir < filas1.nPessoasEspera ) {
                printf("A pessoa com o id %d desistiu da fila 1 porque tinha muita gente a frente.\n", people->id);
                pthread_mutex_unlock(&mutexFilaDeEspera);
                //sprintf(mensagem, "%d-%d-%d-%d", people->id, timestamp, 2, 1);
                //enviarMensagem(mensagem);
                people->desistiu = TRUE;
                return;
            }
            //people->tempoChegadaFiladeEspera = timestamp;
            filas1.nPessoasEspera++;
            pthread_mutex_unlock(&mutexFilaDeEspera);
            // printf("A pessoa com o id %d chegou1 a fila.\n", pessoa->id);
            sem_wait(&filas1.filaEspera);
            pthread_mutex_lock(&mutexVariaveisSimulacao);
            //tempoEspera = minutosDecorridos - people->tempoChegadaFiladeEspera;
            if (tempoEspera > people->tempoMaxEsperaP) { // passou muito tempo à espera, a pessoa desiste
                people->desistiu = TRUE;
                //sem_getvalue(&filas1.FiladeEspera, &valorSemaforo);
                //if (valorSemaforo < filas1.numeroPostosDisponiveis) {
                //sem_post(&filas1.FiladeEspera);
                //}
                printf("A pessoa com o id %d desistiu na fila 1 porque passou muito tempo a espera.\n", people->id);
                pthread_mutex_unlock(&mutexVariaveisSimulacao);
                //sprintf(mensagem, "%d-%d-%d-%d", people->id, tempoEspera, 2, 1);
                //enviarMensagem(mensagem);
                pthread_mutex_lock(&mutexVariaveisSimulacao);
            } 
            
            pthread_mutex_unlock(&mutexVariaveisSimulacao);
            pthread_mutex_lock(&mutexFilaDeEspera);
            // sem_post(&centroTestes1.FiladeEspera);
            filas1.nPessoasEspera--;
            pthread_mutex_unlock(&mutexFilaDeEspera);
        }
    }
    else if (people->fila == 2) {
        printf("A pessoa com o id %d chegou a fila 2.\n", people->id);
    }
    
}


void Pessoa(void *ptr) {
    struct pessoa pessoa = criaPessoa();
    pessoasCriadas[pessoa.id] = &pessoa;
    char mensagem[MAXLINE];

    while (TRUE) {
        FilaDeEspera(&pessoa);
        if (!pessoa.desistiu) {
            // sprintf(mensagem, "%d-%d-%d-%d", pessoa.id, "Z", 3, "Z");
            // enviarMensagem(mensagem);
            //int tipoTeste = -1;
            //int tempoEsperaTeste = 0;
            // printf("TEMPO ESPERA TESTE em ms: %d\n",tempoEsperaTeste);
            //usleep(tempoEsperaTeste);
            //fazerTeste(&pessoa);

            /*if (pessoa.estado)

            if (pessoa.estadoTeste == POSITIVO) {
                printf(AMARELO "Pessoa %d testou positivo \n" RESET, pessoa.id);
                sprintf(mensagem, "%d-%d-%d-%d", pessoa.id, "Z", 8, "Z");
                enviarMensagem(mensagem);
                sem_init(&pessoa.semaforoPessoa, 0, 0);
                pessoa.numeroDiasDesdePositivo = 0;
                pessoa.estado = ISOLAMENTO;
                break;
            } 
            else if (pessoa.estadoTeste == NEGATIVO) {
                printf(AMARELO "Pessoa %d testou negativo \n" RESET, pessoa.id);
                sprintf(mensagem, "%d-%d-%d-%d", pessoa.id, "Z", 9, 0);
                enviarMensagem(mensagem);
                pessoa.estado = SOBREVIVEU;
                break;
            } 
            else {
                printf(AMARELO "Pessoa %d testou inconclusivo \n" RESET, pessoa.id);
                sprintf(mensagem, "%d-%d-%d-%d", pessoa.id, "Z", 12, "Z");
                enviarMensagem(mensagem);
                pessoa.estado = ESPERA;
            }*/
        } 
        else {
            break;
        }
    }
    /*
    if (pessoa.estadoTeste == POSITIVO) {
        pessoa.numeroDiasDesdePositivo = 0;
        if (pessoa.vip ||
            probabilidade(
                configuracao
                    .probabilidadeNaoIdosoPrecisaHospital)) { // Vai para o Hospital
        pessoa.estado = HOSPITAL;
        printf(CIANO
                "A pessoa com id %d foi transportada para o hospital.\n" RESET,
                pessoa.id);
        sprintf(mensagem, "%d-%d-%d-%d", pessoa.id, "Z", 4, "Z");
        enviarMensagem(mensagem);
        pthread_mutex_lock(&mutexVariaveisHospital);
        if (numeroPacientesNoHospital < config.tamanhoHospital) {
            numeroPacientesNoHospital++;
            sem_post(&semaforoMedicos);
            IDsDoentesNoHospital[indexArraysIDS] = pessoa.id;
            pthread_mutex_unlock(&mutexVariaveisHospital);
            sem_wait(&semaforoDoentes);
            pthread_mutex_lock(&mutexVariaveisHospital);
        }
        pthread_mutex_unlock(&mutexVariaveisHospital);
        }
        // printf("PRESEMAFORO\n");
        sem_wait(&pessoa.semaforoPessoa);
        // printf("POSSEMAFORO\n");
    }*/
}

int readConfiguracao(){     //funcao para ler a configuracao

    FILE* fp = fopen("simulador.conf", "r");    //abre o ficheiro para ler
                                                          
    if(fp != NULL){
       char linha[50];               //guarda a linha                                   
       int valor;                    //guarda o valor do parametro                                             
       char parametro[50];           //guarda o parametro

       while(fgets(linha, sizeof(linha), fp) != NULL) {                      // caso o ficheiro não esteja vazio guarda a linha do ficheiro na variável line                      
            
            sscanf(linha, "%s : %d", parametro , &valor);                            // faz scan da linha
               
            
            if(strcmp(parametro, "tamanhoDiscoteca") == 0)
            {                            
                tamanhoDiscoteca = valor;
            } 
            if(strcmp(parametro, "tempoLimiteSimulacao") == 0)
            {                            
                tempoLimiteSimulacao = valor;
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
            if(strcmp(parametro, "tempoMedioChegada") == 0)
            {                
                tempoMedioChegada = valor;
            }
            if(strcmp(parametro, "tempoMedioEspera") == 0)
            {                            
                tempoMedioEspera = valor;
            }  
            if(strcmp(parametro, "tempoFazerTeste") == 0)
            {                            
                tempoFazerTeste = valor;
            } 
            if(strcmp(parametro, "probSerVIP") == 0)
            {					     
                probSerVIP = valor;
            }
            if(strcmp(parametro, "probDesistir") == 0)					     
            {
                probDesistir = valor;
            }
            if(strcmp(parametro, "probSerMulher") == 0)					     
            {
                probSerMulher = valor;
            }
            if(strcmp(parametro, "probSerHomem") == 0)					     
            {
                probSerHomem = valor;
            }
            if(strcmp(parametro, "probMorrerComa") == 0)
            {
                probMorrerComa = valor;
            }
        }
        /*
        printf( "Tamanho Discoteca: %d \n", tamanhoDiscoteca);	
        printf( "Numero Dancarinos: %d \n", numeroDancarinos);	
        printf( "Tamanho Max Fila 1: %d \n", tamanhoMaxFila1);	
        printf( "Tamanho Max Fila 2: %d \n", tamanhoMaxFila2);	
        printf( "Tamanho Max Zona A: %d \n", tamanhoMaxZonaA);	
        printf( "Tamanho Max Zona B: %d \n", tamanhoMaxZonaB);	
        printf( "Tamanho Max Padaria: %d \n", tamanhoMaxPadaria);
        */
        printf("Tempo Limite Simulacao: %d \n", tempoLimiteSimulacao);	
        
        fclose(fp);
    }
    else {
        printf("erro: nao foi possivel ler o ficheiro de configuracao. \n");
    }
}


int simulacao() {


    while (tempoLimiteSimulacao != tempoDecorrido)
    {
        tempoDecorrido++;
        printf("limite: %d \n", tempoLimiteSimulacao);
        printf("decorrido: %d \n", tempoDecorrido);
        /*
        if (TRUE == 1) {
          // cria tarefas pessoas
          pthread_mutex_unlock(&mutexVariaveisSimulacao);
          if (pthread_create(&tasksID[pessoaID], NULL, Pessoa, NULL)) {
            printf("Erro na criação da tarefa\n");
            exit(1);
          }
          pthread_mutex_lock(&mutexVariaveisSimulacao);
        }*/

    }
    

    
}





int main(int argc, char const * argv[]){


    criaSocket();
    readConfiguracao();
    simulacao();
    /*sockfd = criarSocket();
    simulacao(sockfd);
    close(sockfd);*/
    return 0;
}


