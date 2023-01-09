#include "header.h"

//Variaveis globais
int sockfd = 0; // inicia socket
int pessoaID = 1;
int tempoDecorrido = 0; // tempo decorrido em segundos
int nPessoasNaDiscoteca = 0;

//configuracao e filas
struct configuracao config;
struct Fila1 filas1;
struct Fila2 filas2;


//Trincos e Semaforos
pthread_mutex_t mutexCriarPessoa;
pthread_mutex_t mutexFilaDeEspera;
pthread_mutex_t mutexSimulacao;
pthread_mutex_t mutexFumar;
sem_t semaforoEnviaInformacao;
sem_t semaforoFila1;
sem_t semaforoFila2;
sem_t semaforoDiscoteca;
sem_t semaforoJAM;
sem_t semaforoSmokingArea;
sem_t semaforoPadaria;
sem_t semaforoLevarSoro;


//TAREFAS
pthread_t tasksID[SIZE_TASKS]; 
struct pessoa *pessoasCriadas[100000];


int criaSocket() {
    struct sockaddr_un serv_addr;
    int servlen;

	// Cria o socket
    if ((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
        printf("erro: nao foi possivel criar o socket. \n");
    }

    // Zerar o socket
    bzero((char*)&serv_addr, sizeof(serv_addr));

    // Familia do socket
    serv_addr.sun_family = AF_UNIX;
    strcpy(serv_addr.sun_path, UNIXSTR_PATH);
    
    servlen = strlen(serv_addr.sun_path) + sizeof(serv_addr.sun_family);

    // Estabelecer a ligacao com o socket
    bool booleano = false;
    while (connect(sockfd, (struct sockaddr *)&serv_addr, servlen) < 0) {
        if (booleano == false) {
            printf("Espera pelo monitor...\n");
            booleano = true;
        }
    }
 
    if (sockfd < 0) {
    	perror("erro: nao foi possivel aceitar pedido.\n");
    }

    printf("Simulador pronto. \n");
    return sockfd;

}

// funcao que envia mensagens para o monitor
void enviaInformacao(int sockfd, int pessoa_id, int tempoMedido, int estado, int acontecimento, int zona) {
    sem_wait(&semaforoEnviaInformacao);
    char buffer[MAXLINE];
    int numero = 0;
    sprintf(buffer, "%d %d %d %d %d", pessoa_id, tempoMedido, estado, acontecimento, zona);
    numero = strlen(buffer) + 1;    
    if(send(sockfd, buffer, numero, 0) != numero){
        perror("erro: nao foi possivel enviar os dados. \n");   
    }
    usleep(10000);
    sem_post(&semaforoEnviaInformacao);
}


//<<<<<<<<<<<<<<<<<<<<<<<< F.AUX >>>>>>>>>>>>>>>>>>>>>>>>>>>>>

int probabilidade (float valor) {
	return (randomNumber(100, 0)) < (valor * 100);
}

int randomNumber(int max, int min) {
	return rand() % (max + 1 - min) + min;
}

char *defineTipoPessoa(struct pessoa *people){
    char *tipoPessoa = malloc(35);

    if(people->vip == 1){
        if(people->sexualidade == MULHER) {
            sprintf(tipoPessoa, "A Mulher VIP com o id %d", people->id);
            return tipoPessoa;
        }
        else if (people->sexualidade == HOMEM){
            sprintf(tipoPessoa, "O Homem VIP com o id %d", people->id);
            return tipoPessoa;
        }
    }
    else {
        if(people->sexualidade == MULHER) {
            sprintf(tipoPessoa, "A Mulher com o id %d", people->id);
            return tipoPessoa;
        }
        else if (people->sexualidade == HOMEM){
            sprintf(tipoPessoa, "O Homem com o id %d", people->id);
            return tipoPessoa;
        }
    }

}


//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

struct pessoa criaPessoa() {

    pthread_mutex_lock(&mutexCriarPessoa);

    struct pessoa people;
    
    people.id = pessoaID;
    people.sexualidade = randomNumber(HOMEM, MULHER);
    people.fila = 1;
    people.zona = JAM;
    people.vip = probabilidade(config.probSerVIP);
    //people.vip = 0;
    people.desistir = FALSE;

    if (people.vip == TRUE) {
        people.fila = 2;
    }
    if (people.fila == 1) {
        people.nPessoasAFrenteDesistir = randomNumber((config.tamanhoMaxFila1 / 3), (config.tamanhoMaxFila1 / 4));
    } 
    else if (people.fila == 2) {
        people.nPessoasAFrenteDesistir = randomNumber((config.tamanhoMaxFila2 / 3), (config.tamanhoMaxFila2 / 4));
    }
    people.estado = ESPERA;
    people.tempoMaxEsperaP = randomNumber(config.tempoMaxEspera, (config.tempoMaxEspera * 2) / 3);

    pessoaID++;
    pthread_mutex_unlock(&mutexCriarPessoa);
    return people;
}

void FilaDeEspera(struct pessoa *people) {

    int timestamp = tempoDecorrido;
    int waitTime1, waitTime2;
    char *tipoDePessoa;

    int semaforo;
    int semDisco;
    int semJAM;
    int semSmokingArea;
    int semPadaria;

    if (people->fila == 2) {
        if (nPessoasNaDiscoteca != config.tamanhoMaxJam) {
            sem_wait(&semaforoJAM);
            tipoDePessoa = defineTipoPessoa(people);
            printf( GREEN "%s já entrou na discoteca e vai para o JAM \n" STOP, tipoDePessoa);
            free(tipoDePessoa);
            nPessoasNaDiscoteca++;
            enviaInformacao(sockfd, people->id, timestamp, 4, 2, people->zona);
            people->estado = ENTROU;
        }
        else
        {
            pthread_mutex_lock(&mutexFilaDeEspera);
            int nPessoasNaFila2 = filas2.nPessoasFila2;
            pthread_mutex_unlock(&mutexFilaDeEspera);

            // Se o numero de pessoas na fila de espera for menor que o tamanho da fila avança
            if (nPessoasNaFila2 < config.tamanhoMaxFila2) {
                sem_wait(&semaforoFila2);
                tipoDePessoa = defineTipoPessoa(people);
                printf( CYAN "%s chegou a fila 2, as %d \n" STOP, tipoDePessoa, timestamp);
                free(tipoDePessoa);
                enviaInformacao(sockfd, people->id, timestamp, 0, 2, people->zona); 
                
                //O numero de pessoas na fila é superior ao numero que a pessoa admite ter a sua frente, entao ela desiste
                //muita gente na fila logo desiste
                if (people->nPessoasAFrenteDesistir < nPessoasNaFila2) {
                    sem_post(&semaforoFila2);
                    pthread_mutex_lock(&mutexFilaDeEspera);
                    filas2.nPessoasFila2--;
                    pthread_mutex_unlock(&mutexFilaDeEspera);
                    tipoDePessoa = defineTipoPessoa(people);
                    printf(YELLOW "%s desistiu da fila do 2 porque tinha muita gente a frente.\n" STOP, tipoDePessoa);
                    free(tipoDePessoa);
                    enviaInformacao(sockfd, people->id, timestamp, 1, 2, people->zona);   
                    people->desistir = TRUE;
                }
                //tem fila pequena logo fica a espera
                else {
                    sem_wait(&semaforoFila2);
                    pthread_mutex_lock(&mutexFilaDeEspera);
                    filas2.nPessoasFila2++;
                    pthread_mutex_unlock(&mutexFilaDeEspera);
                    pthread_mutex_lock(&mutexSimulacao);
                    waitTime2 = (randomNumber((config.tempoMedioChegada * 2), (config.tempoMedioEspera)));
                    pthread_mutex_unlock(&mutexSimulacao);
                    people->zona = JAM;
                    //se tempo que a pessoa vai esperar é maior que o tempo maximo que ele deseja esperar, esta desiste
                    //esta a demorar muito tempo logo desiste
                    if (waitTime2 > people->tempoMaxEsperaP) {
                        sem_post(&semaforoFila2);
                        pthread_mutex_lock(&mutexFilaDeEspera);
                        filas2.nPessoasFila2--;
                        pthread_mutex_unlock(&mutexFilaDeEspera);
                        tipoDePessoa = defineTipoPessoa(people);
                        printf( YELLOW "%s desistiu da fila do 2 porque esteve muito tempo a espera.\n" STOP, tipoDePessoa);
                        free(tipoDePessoa);
                        enviaInformacao(sockfd, people->id, timestamp, 1, 2, people->zona); 
                        people->desistir = TRUE;
                    }
                    //nao esta a demorar muito tempo logo espera e vai entrar na discoteca
                    else {
                        sem_wait(&semaforoJAM);
                        sem_post(&semaforoFila2);
                        pthread_mutex_lock(&mutexFilaDeEspera);
                        filas2.nPessoasFila2--;
                        pthread_mutex_unlock(&mutexFilaDeEspera);
                        tipoDePessoa = defineTipoPessoa(people);
                        printf(GREEN "%s entrou na discoteca e vai para o JAM \n" STOP, tipoDePessoa);
                        free(tipoDePessoa);
                        enviaInformacao(sockfd, people->id, timestamp, 2, 2, people->zona);
                        people->estado = ENTROU; 
                    }
                }
            }
            else {
                sem_post(&semaforoFila2);
                pthread_mutex_unlock(&mutexFilaDeEspera);
                people->desistir = TRUE;
            }
        }
    }
    if (people->fila == 1) {
        if (nPessoasNaDiscoteca != config.tamanhoMaxJam) {
            sem_wait(&semaforoJAM);
            tipoDePessoa = defineTipoPessoa(people);
            printf( GREEN "%s já entrou na discoteca e vai para o JAM \n" STOP, tipoDePessoa);
            free(tipoDePessoa);
            nPessoasNaDiscoteca++;
            enviaInformacao(sockfd, people->id, timestamp, 4, 1, people->zona);
            people->estado = ENTROU;
        }
        else
        {
            pthread_mutex_lock(&mutexFilaDeEspera);
            int nPessoasNaFila1 = filas1.nPessoasFila1;
            pthread_mutex_unlock(&mutexFilaDeEspera);

            // Se o numero de pessoas na fila de espera for menor que o tamanho da fila avança
            if (nPessoasNaFila1 < config.tamanhoMaxFila1) {
                sem_wait(&semaforoFila1);
                tipoDePessoa = defineTipoPessoa(people);
                printf( CYAN "%s chegou a fila 1, as %d \n" STOP, tipoDePessoa, timestamp);
                free(tipoDePessoa);
                enviaInformacao(sockfd, people->id, timestamp, 0, 1, people->zona); 
                
                //O numero de pessoas na fila é superior ao numero que a pessoa admite ter a sua frente, entao ela desiste
                //muita gente na fila logo desiste
                if (people->nPessoasAFrenteDesistir < nPessoasNaFila1) {
                    sem_post(&semaforoFila1);
                    pthread_mutex_lock(&mutexFilaDeEspera);
                    filas1.nPessoasFila1--;
                    pthread_mutex_unlock(&mutexFilaDeEspera);
                    tipoDePessoa = defineTipoPessoa(people);
                    printf(YELLOW "%s desistiu da fila do 1 porque tinha muita gente a frente.\n" STOP, tipoDePessoa);
                    free(tipoDePessoa);
                    enviaInformacao(sockfd, people->id, timestamp, 1, 1, people->zona);   
                    people->desistir = TRUE;
                }
                //tem fila pequena logo fica a espera
                else {
                    sem_wait(&semaforoFila1);
                    pthread_mutex_lock(&mutexFilaDeEspera);
                    filas1.nPessoasFila1++;
                    pthread_mutex_unlock(&mutexFilaDeEspera);
                    pthread_mutex_lock(&mutexSimulacao);
                    waitTime1 = (randomNumber((config.tempoMedioChegada * 2), (config.tempoMedioEspera)));
                    pthread_mutex_unlock(&mutexSimulacao);
                    people->zona = JAM;
                    //se tempo que a pessoa vai esperar é maior que o tempo maximo que ele deseja esperar, esta desiste
                    //esta a demorar muito tempo logo desiste
                    if (waitTime1 > people->tempoMaxEsperaP) {
                        sem_post(&semaforoFila1);
                        pthread_mutex_lock(&mutexFilaDeEspera);
                        filas1.nPessoasFila1--;
                        pthread_mutex_unlock(&mutexFilaDeEspera);
                        tipoDePessoa = defineTipoPessoa(people);
                        printf( YELLOW "%s desistiu da fila do 1 porque esteve muito tempo a espera.\n" STOP, tipoDePessoa);
                        free(tipoDePessoa);
                        enviaInformacao(sockfd, people->id, timestamp, 1, 1, people->zona); 
                        people->desistir = TRUE;
                    }
                    //nao esta a demorar muito tempo logo espera e vai entrar na discoteca
                    else {
                        sem_wait(&semaforoJAM);
                        sem_post(&semaforoFila1);
                        pthread_mutex_lock(&mutexFilaDeEspera);
                        filas1.nPessoasFila1--;
                        pthread_mutex_unlock(&mutexFilaDeEspera);
                        tipoDePessoa = defineTipoPessoa(people);
                        printf(GREEN "%s entrou na discoteca e vai para o JAM\n" STOP, tipoDePessoa);
                        free(tipoDePessoa);
                        enviaInformacao(sockfd, people->id, timestamp, 2, 1, people->zona);
                        people->estado = ENTROU; 
                    }
                }
            }
            else {
                pthread_mutex_unlock(&mutexFilaDeEspera);
                people->desistir = TRUE;
            }
        }
    }
}

void Pessoa(void *ptr) {
    struct pessoa people = criaPessoa();
    pessoasCriadas[people.id] = &people;   
    
    int timestamp = tempoDecorrido;
    
    while (TRUE) {
        FilaDeEspera(&people);
        sleep(config.tamanhoMaxJam);
        if (people.desistir == FALSE) {
            if (people.estado == ENTROU) {
                
                if (probabilidade(config.probMudarZona) == 1) {
                    printf("A pessoa com o id %d quer mudar de zona \n", people.id);
                    if(people.zona == JAM){
                        people.zona = randomNumber(PADARIA, SMOKING_AREA);
                        if (people.zona == SMOKING_AREA) {
                            sem_post(&semaforoJAM);
                            sem_wait(&semaforoSmokingArea);
                            printf( YELLOW "A pessoa com o id %d muda do JAM para a Smoking Area. \n" STOP, people.id);
                            enviaInformacao(sockfd, people.id, timestamp, 7, JAM, SMOKING_AREA);
                        }
                        else if (people.zona == PADARIA) {
                            sem_post(&semaforoJAM);
                            sem_wait(&semaforoPadaria);
                            printf( YELLOW "A pessoa com o id %d muda do JAM para a Padaria. \n" STOP, people.id);
                            enviaInformacao(sockfd, people.id, timestamp, 7, JAM, PADARIA);
                        }
                    }
                    if (people.zona == SMOKING_AREA) {
                        if (probabilidade(config.probIr) == 1) {
                            sem_wait(&semaforoSmokingArea);
                            if (people.vip == TRUE) {
                                pthread_mutex_lock(&mutexFumar);
                                printf( BLUE "A pessoa VIP com o id %d foi fumar. \n" STOP, people.id);
                                enviaInformacao(sockfd, people.id, timestamp, 11, 1, people.zona);
                                sleep(config.tempoFumar);
                                printf( BLUE "A pessoa VIP com o id %d acabou de fumar e foi-se embora. \n" STOP, people.id);
                                enviaInformacao(sockfd, people.id, timestamp, 12, 1, people.zona);
                                pthread_mutex_unlock(&mutexFumar);
                                people.desistir == TRUE;
                            }
                            else if (people.vip == FALSE) {
                                pthread_mutex_lock(&mutexFumar);
                                printf( BLUE "A pessoa com o id %d foi fumar. \n" STOP, people.id);
                                pthread_mutex_unlock(&mutexFumar);
                                enviaInformacao(sockfd, people.id, timestamp, 11, 1, people.zona);
                                sleep(config.tempoFumar);   
                                pthread_mutex_lock(&mutexFumar);    
                                printf( BLUE "A pessoa com o id %d acabou de fumar e foi-se embora. \n" STOP, people.id);
                                pthread_mutex_unlock(&mutexFumar);
                                enviaInformacao(sockfd, people.id, timestamp, 12, 1, people.zona);
                                people.desistir == TRUE;
                                
                            }
                        }
                        else {
                            people.zona = PADARIA;
                            if (people.zona == PADARIA) {
                                sem_post(&semaforoSmokingArea);
                                sem_wait(&semaforoPadaria);
                                printf( YELLOW "A pessoa com o id %d muda da Smoking Area para a Padaria. \n" STOP, people.id);
                                enviaInformacao(sockfd, people.id, timestamp, 7, SMOKING_AREA, PADARIA);
                            }
                        }
                    }
                    if (people.zona == PADARIA) {
                        if (probabilidade(config.probIr) == 1) {
                            sem_wait(&semaforoPadaria);
                            printf( BLUE "A pessoa com o id %d está a comer. \n" STOP, people.id);
                            enviaInformacao(sockfd, people.id, timestamp, 9, 1, people.zona);
                            sleep(config.tempoComer);
                            sem_post(&semaforoPadaria);
                            printf( BLUE "A pessoa com o id %d acabou de comer e foi-se embora. \n" STOP, people.id);
                            enviaInformacao(sockfd, people.id, timestamp, 10, 1, people.zona);
                            people.desistir == TRUE;
                        }
                        else {                            
                            people.zona = randomNumber(SMOKING_AREA, JAM);
                            if (people.zona == JAM) {
                                sem_post(&semaforoPadaria);
                                sem_wait(&semaforoJAM);
                                printf( YELLOW "A pessoa com o id %d muda da Padaria para a JAM. \n" STOP, people.id);
                                enviaInformacao(sockfd, people.id, timestamp, 7, PADARIA, JAM);
                            }
                            if (people.zona == SMOKING_AREA) {
                                sem_post(&semaforoPadaria);
                                sem_wait(&semaforoSmokingArea);                
                                printf( YELLOW "A pessoa com o id %d muda da Padaria para a Smoking Area. \n" STOP, people.id);
                                enviaInformacao(sockfd, people.id, timestamp, 7, PADARIA, SMOKING_AREA);
                            }
                        }
                    }
                }
                else if (probabilidade(config.probMudarZona) == 0){
                    printf("A pessoa com o id %d nao quer mudar de zona \n", people.id);
                    if (probabilidade(config.probEntrarComa) == 1) { 
                        if (people.zona == JAM) {
                            sem_post(&semaforoJAM);
                        }
                        if (people.zona == SMOKING_AREA) {
                            sem_post(&semaforoSmokingArea);
                        }
                        if (people.zona == PADARIA) {
                            sem_post(&semaforoPadaria);
                        }
                        people.estado == COMA;
                        printf( RED "A pessoa com o id %d bebeu demasiado e entrou em coma. \n" STOP, people.id);
                        enviaInformacao(sockfd, people.id, timestamp, 3, 1, people.zona);
                        sem_wait(&semaforoLevarSoro);
                        printf( BLUE "A pessoa com o id %d está a levar soro para curar do coma alcoolico. \n" STOP, people.id);
                        enviaInformacao(sockfd, people.id, timestamp, 8, 1, people.zona);
                        sleep(config.tempoLevarSoro);

                        if (probabilidade(config.probMorrerComa) == 1){
                            sem_post(&semaforoLevarSoro);
                            printf( RED "A pessoa com o id %d não aguentou e infelizmente morreu. \n" STOP, people.id);
                            enviaInformacao(sockfd, people.id, timestamp, 6, 1, people.zona);
                            people.desistir == TRUE;
                        }
                        else {
                            sem_post(&semaforoLevarSoro);
                            printf( RED "A pessoa com o id %d felizmente aguentou o tratamento e sobreviveu. \n" STOP, people.id);
                            enviaInformacao(sockfd, people.id, timestamp, 5, 1, people.zona);
                            people.desistir == TRUE;
                        }     
                    }
                    else {
                        if (people.zona == JAM) {
                            sem_post(&semaforoJAM);
                        }
                        if (people.zona == SMOKING_AREA) {
                            sem_post(&semaforoSmokingArea);
                        }
                        if (people.zona == PADARIA) {
                            sem_post(&semaforoPadaria);
                        }
                        printf(PURPLE "A pessoa com o id %d cansou-se e saiu da discoteca. \n" STOP, people.id);
                        enviaInformacao(sockfd, people.id, timestamp, 1, 3, people.zona);
                        people.desistir == TRUE;
                    } 
                }
            }
        } 
        else {
            break;
        }    
    }
}

//funcao para ler o ficheiro de configuracao
int readConfiguracao(char ficheiro[]) {     

    FILE* fp;
    fp = fopen(ficheiro, "r");    //abre o ficheiro e le

    if(fp != NULL){ // se o ficheiro nao estiver vazio continua a leitura
        fseek(fp, 0, SEEK_END);
        long tamFicheiro = ftell(fp);
        rewind(fp);
        char buffer[tamFicheiro];
        fread(buffer, 1, tamFicheiro, fp);
        fclose(fp);

        int counter = 0;
        char *paragrafo = strtok(buffer, "\n");
        char *linhas[25];
        while (paragrafo != NULL) {
            linhas[counter++] = paragrafo;
            paragrafo = strtok(NULL, "\n");
        }
        char *array[2];
        char *value[18];
        for (int i = 0; i < 18; i++) {
            char *sinal = strtok(linhas[i], "->");
            counter = 0;
            while (sinal != NULL) {
                array[counter++] = sinal;
                sinal = strtok(NULL, "->");
            }
            value[i] = array[1];
        } 
        config.tamanhoDiscoteca = (int)strtol(value[0], NULL, 10);
        config.tempoLimiteSimulacao = strtol(value[1], NULL, 10);
        config.tamanhoMaxFila1 = strtol(value[2], NULL, 10);
        config.tamanhoMaxFila2 = strtol(value[3], NULL, 10);
        config.tamanhoMaxJam = strtol(value[4], NULL, 10);
        config.tamanhoMaxSmokingArea = strtol(value[5], NULL, 10);
        config.tamanhoMaxPadaria = strtol(value[6], NULL, 10);
        config.tempoMedioChegada = strtol(value[7], NULL, 10);
        config.tempoMedioEspera = strtol(value[8], NULL, 10);
        config.tempoMaxEspera = strtol(value[9], NULL, 10);
        config.tempoLevarSoro = strtol(value[10], NULL, 10);
        config.tempoComer = strtol(value[11], NULL, 10);
        config.tempoFumar = strtol(value[12], NULL, 10);
        config.probSerVIP = strtof(value[13], NULL);
        config.probEntrarComa = strtof(value[14], NULL);
        config.probMorrerComa = strtof(value[15], NULL);
        config.probIr = strtof(value[16], NULL);
        config.probMudarZona = strtof(value[17], NULL);
    }
    else {
        perror("erro: ficheiro nao existe. \n");  
    }   
}


int simulacao(char* configFile) {

    readConfiguracao(configFile);
    semaforosTrincos();

    while (config.tempoLimiteSimulacao != tempoDecorrido)
    {
        tempoDecorrido++;
        if (tempoDecorrido % config.tempoMedioChegada == 0) {
            // cria tarefas pessoas
            pthread_mutex_unlock(&mutexSimulacao);
            if (pthread_create(&tasksID[pessoaID], NULL, Pessoa, NULL)) {
                printf("Erro na criação da tarefa\n");
                exit(1);
            }
            pthread_mutex_lock(&mutexSimulacao);    
            sleep(1);      
        }
        if (config.tempoLimiteSimulacao == tempoDecorrido) {
            enviaInformacao(sockfd, 0, 0, 99, 0, 0); 
        }
    }
    
}



void semaforosTrincos() {        
    if (pthread_mutex_init(&mutexSimulacao, NULL) != 0) {
        printf("Inicializacao do trinco falhou.\n");
    }
    if (pthread_mutex_init(&mutexCriarPessoa, NULL) != 0) {
        printf("Inicializacao do trinco falhou.\n");
    }
    if (pthread_mutex_init(&mutexFilaDeEspera, NULL) != 0) {
        printf("Inicializacao do trinco falhou.\n");
    }
    if (pthread_mutex_init(&mutexFumar, NULL) != 0) {
        printf("Inicializacao do trinco falhou.\n");
    }
    sem_init(&semaforoFila1, 0, config.tamanhoMaxFila1);
    sem_init(&semaforoFila2, 0, config.tamanhoMaxFila2);
    sem_init(&semaforoDiscoteca, 0, config.tamanhoDiscoteca);
    sem_init(&semaforoJAM, 0, config.tamanhoMaxJam);
    sem_init(&semaforoSmokingArea, 0, config.tamanhoMaxSmokingArea);
    sem_init(&semaforoPadaria, 0, config.tamanhoMaxPadaria);
    sem_init(&semaforoLevarSoro, 0, 2);
    sem_init(&semaforoEnviaInformacao, 0, 1);
}


int main(int argc, char const * argv[]){

    sockfd = criaSocket();
    simulacao(argv[1]);
    close(sockfd);
    return 0;
}


