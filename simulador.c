#include "header.h"

//Variaveis globais
int sockfd = 0; // inicia socket
int pessoaID = 1;
int tempoDecorrido = 0; // tempo decorrido em segundos
int minutosPassados = 0;
int nPessoasNaDiscoteca = 0;
int nPessoasNaZonaA = 0;

//configuracao e filas
struct configuracao config;
struct Fila1 filas1;
struct Fila2 filas2;
struct ZonaA zona_A;
struct ZonaB zona_B;
struct Padaria padaria;


//Trincos e Semaforos
pthread_mutex_t mutexPessoasDisco;
pthread_mutex_t mutexCriarPessoa;
pthread_mutex_t mutexFilaDeEspera;
pthread_mutex_t mutexVariaveisSimulacao;
sem_t semaforoEnviaInformacao;
sem_t semaforoFila1;
sem_t semaforoFila2;
sem_t semaforoDiscoteca;
sem_t semaforoZonaA;
sem_t semaforoZonaB;
sem_t semaforoPadaria;


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

char *defineZona(struct pessoa *people){
    char *zona = malloc(35);
    
    if(people->zona == ZONA_A){
        sprintf(zona, "vai para a Zona A.");
        return zona;
    }
    else if (people->zona == ZONA_B) {
        sprintf(zona, "vai para a Zona B.");
        return zona;
    }
    else if (people->zona == PADARIA) {
        sprintf(zona, "vai direto para a Padaria.");
        return zona;
    }
    
}

void TesteComa(struct pessoa *people){
    if (probabilidade(config.probMorrerComa) == 1){
        people->teste == MORREU;
    }
    else {
        people->teste == SOBREVIVEU;
    }
}

char *defineNovaZona(struct pessoa *people){
    char *novaZona = malloc(35);
    
    if(people->zona == ZONA_A){
        people->zona = randomNumber(PADARIA, ZONA_B);
        if (people->zona == ZONA_B) {
            sem_wait(&semaforoZonaB);
            sprintf(novaZona, "Zona B");
            enviaInformacao(sockfd, people->id, 0, 7, ZONA_A, ZONA_B);
            return novaZona;
        }
        else if (people->zona == PADARIA) {
            sem_wait(&semaforoPadaria);
            sprintf(novaZona, "Padaria");
            enviaInformacao(sockfd, people->id, 0, 7, ZONA_A, PADARIA);
            return novaZona;
        }
    }
    else if (people->zona == ZONA_B) {
        people->zona = ZONA_A;
        if (people->zona == ZONA_A) {
            sem_wait(&semaforoZonaA);
            sprintf(novaZona, "Zona A");
            enviaInformacao(sockfd, people->id, 0, 7, ZONA_B, ZONA_A);
            return novaZona;
        }
    }
    else if (people->zona == PADARIA) {
        people->zona = randomNumber(ZONA_B, ZONA_A);
        if (people->zona == ZONA_A) {
            sem_wait(&semaforoZonaA);
            sprintf(novaZona, "Zona A");
            enviaInformacao(sockfd, people->id, 0, 7, PADARIA, ZONA_A);
            return novaZona;
        }
        else if (people->zona == ZONA_B) {
            sem_wait(&semaforoZonaB);
            sprintf(novaZona, "Zona B");
            enviaInformacao(sockfd, people->id, 0, 7, PADARIA, ZONA_B);
            return novaZona;
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
    people.zona = ZONA_A;
    people.vip = probabilidade(config.probSerVIP);
    //people.vip = 0;
    people.desistiu = FALSE;

    if (people.vip == 1) {
        people.fila = 2;
    }
    if (people.fila == 1) {
        people.nPessoasAFrenteDesistir = randomNumber((config.tamanhoMaxFila1 / 3), (config.tamanhoMaxFila1 / 4));
    } 
    else if (people.fila == 2) {
        people.nPessoasAFrenteDesistir = randomNumber((config.tamanhoMaxFila2 / 3), (config.tamanhoMaxFila2 / 4));
    }
    people.naFila = TRUE;
    people.estado = ESPERA;
    people.teste = NAO_FEZ_TESTE;
    people.tempoMaxEsperaP = randomNumber(config.tempoMaxEspera, (config.tempoMaxEspera * 2) / 3);

    pessoaID++;
    pthread_mutex_unlock(&mutexCriarPessoa);
    return people;
}

void FilaDeEspera(struct pessoa *people) {

    int timestamp = tempoDecorrido;
    int waitTime1, waitTime2;
    int tempoNaFila1 = 0;
    int tempoNaFila2 = 0;
    char *tipoDePessoa;
    char *zonaP;

    int semaforo;
    int semDisco;
    int semZonaA;
    int semZonaB;
    int semPadaria;

    if (people->fila == 1) {
        if (nPessoasNaDiscoteca != config.tamanhoMaxZonaA) {
            sem_wait(&semaforoZonaA);
            tipoDePessoa = defineTipoPessoa(people);
            zonaP = defineZona(people);
            printf( GREEN "%s já entrou na discoteca e %s \n" STOP, tipoDePessoa, zonaP);
            free(tipoDePessoa);
            free(zonaP);
            nPessoasNaDiscoteca++;
            enviaInformacao(sockfd, people->id, timestamp, 4, 1, people->zona);
            people->estado = ENTROU;
        }
        else
        {
            pthread_mutex_lock(&mutexFilaDeEspera);
            int nPessoasNaFila1 = zona_A.nPessoasFila1;
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
                    zona_A.nPessoasFila1--;
                    pthread_mutex_unlock(&mutexFilaDeEspera);
                    tipoDePessoa = defineTipoPessoa(people);
                    printf(YELLOW "%s desistiu da fila do 1 porque tinha muita gente a frente.\n" STOP, tipoDePessoa);
                    free(tipoDePessoa);
                    enviaInformacao(sockfd, people->id, timestamp, 1, 1, people->zona);   
                    people->desistiu = TRUE;
                }
                //tem fila pequena logo fica a espera
                else {
                    pthread_mutex_lock(&mutexFilaDeEspera);
                    zona_A.nPessoasFila1++;
                    pthread_mutex_unlock(&mutexFilaDeEspera);
                    pthread_mutex_lock(&mutexVariaveisSimulacao);
                    waitTime1 = (randomNumber((config.tempoMedioChegada * 2), (config.tempoMedioEspera)));
                    pthread_mutex_unlock(&mutexVariaveisSimulacao);
                    
                    //se tempo que a pessoa vai esperar é maior que o tempo maximo que ele deseja esperar, esta desiste
                    //esta a demorar muito tempo logo desiste
                    if (waitTime1 > people->tempoMaxEsperaP) {
                        sem_post(&semaforoFila1);
                        pthread_mutex_lock(&mutexFilaDeEspera);
                        zona_A.nPessoasFila1--;
                        pthread_mutex_unlock(&mutexFilaDeEspera);
                        tipoDePessoa = defineTipoPessoa(people);
                        printf( YELLOW "%s desistiu da fila do 1 porque esteve muito tempo a espera.\n" STOP, tipoDePessoa);
                        free(tipoDePessoa);
                        enviaInformacao(sockfd, people->id, timestamp, 1, 1, people->zona); 
                        people->desistiu = TRUE;
                    }
                    //nao esta a demorar muito tempo logo espera e vai entrar na discoteca
                    else {
                        sem_post(&semaforoFila1);
                        sem_wait(&semaforoZonaA);
                        pthread_mutex_lock(&mutexFilaDeEspera);
                        zona_A.nPessoasFila1--;
                        pthread_mutex_unlock(&mutexFilaDeEspera);
                        tipoDePessoa = defineTipoPessoa(people);
                        zonaP = defineZona(people);
                        printf(GREEN "%s entrou na discoteca e %s \n" STOP, tipoDePessoa, zonaP);
                        free(tipoDePessoa);
                        free(zonaP);
                        enviaInformacao(sockfd, people->id, timestamp, 2, 1, people->zona);
                        people->estado = ENTROU; 
                    }
                }
            }
            else {
                pthread_mutex_unlock(&mutexFilaDeEspera);
                people->desistiu = TRUE;
            }
        }
    }
    if (people->fila == 2) {
        if (nPessoasNaDiscoteca != config.tamanhoMaxZonaA) {
            sem_wait(&semaforoZonaA);
            tipoDePessoa = defineTipoPessoa(people);
            zonaP = defineZona(people);
            printf( GREEN "%s já entrou na discoteca e %s \n" STOP, tipoDePessoa, zonaP);
            free(tipoDePessoa);
            free(zonaP);
            nPessoasNaDiscoteca++;
            enviaInformacao(sockfd, people->id, timestamp, 4, 2, people->zona);
            people->estado = ENTROU;
        }
        else
        {
            pthread_mutex_lock(&mutexFilaDeEspera);
            int nPessoasNaFila2 = zona_A.nPessoasFila2;
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
                    zona_A.nPessoasFila2--;
                    pthread_mutex_unlock(&mutexFilaDeEspera);
                    tipoDePessoa = defineTipoPessoa(people);
                    printf(YELLOW "%s desistiu da fila do 2 porque tinha muita gente a frente.\n" STOP, tipoDePessoa);
                    free(tipoDePessoa);
                    enviaInformacao(sockfd, people->id, timestamp, 1, 2, people->zona);   
                    people->desistiu = TRUE;
                }
                //tem fila pequena logo fica a espera
                else {
                    pthread_mutex_lock(&mutexFilaDeEspera);
                    zona_A.nPessoasFila2++;
                    pthread_mutex_unlock(&mutexFilaDeEspera);
                    pthread_mutex_lock(&mutexVariaveisSimulacao);
                    waitTime2 = (randomNumber((config.tempoMedioChegada * 2), (config.tempoMedioEspera)));
                    pthread_mutex_unlock(&mutexVariaveisSimulacao);
                    
                    //se tempo que a pessoa vai esperar é maior que o tempo maximo que ele deseja esperar, esta desiste
                    //esta a demorar muito tempo logo desiste
                    if (waitTime2 > people->tempoMaxEsperaP) {
                        sem_post(&semaforoFila2);
                        pthread_mutex_lock(&mutexFilaDeEspera);
                        zona_A.nPessoasFila2--;
                        pthread_mutex_unlock(&mutexFilaDeEspera);
                        tipoDePessoa = defineTipoPessoa(people);
                        printf( YELLOW "%s desistiu da fila do 2 porque esteve muito tempo a espera.\n" STOP, tipoDePessoa);
                        free(tipoDePessoa);
                        enviaInformacao(sockfd, people->id, timestamp, 1, 2, people->zona); 
                        people->desistiu = TRUE;
                    }
                    //nao esta a demorar muito tempo logo espera e vai entrar na discoteca
                    else {
                        sem_post(&semaforoFila2);
                        sem_wait(&semaforoZonaA);
                        pthread_mutex_lock(&mutexFilaDeEspera);
                        zona_A.nPessoasFila2--;
                        pthread_mutex_unlock(&mutexFilaDeEspera);
                        tipoDePessoa = defineTipoPessoa(people);
                        zonaP = defineZona(people);
                        printf(GREEN "%s entrou na discoteca e %s \n" STOP, tipoDePessoa, zonaP);
                        free(tipoDePessoa);
                        free(zonaP);
                        enviaInformacao(sockfd, people->id, timestamp, 2, 2, people->zona);
                        people->estado = ENTROU; 
                    }
                }
            }
            else {
                pthread_mutex_unlock(&mutexFilaDeEspera);
                people->desistiu = TRUE;
            }
        }
    }
}

void Pessoa(void *ptr) {
    struct pessoa people = criaPessoa();
    pessoasCriadas[people.id] = &people;   
    
    int timestamp = tempoDecorrido;
    char *newZona;
    
    while (TRUE) {
        FilaDeEspera(&people);
        sleep(config.tamanhoMaxZonaA);
        if (people.desistiu == FALSE) {
            if (people.estado == ENTROU) {
                
                if (probabilidade(config.probMudarZona) == 1) {
                    if(people.zona == ZONA_A){
                        sem_post(&semaforoZonaA);
                        newZona = defineNovaZona(&people);
                        printf( PURPLE "A pessoa com o id %d muda da Zona A para a %s. \n" STOP, people.id, newZona);
                        free(newZona);
                    }
                    if (people.zona == ZONA_B) {
                        sem_post(&semaforoZonaB);
                        newZona = defineNovaZona(&people);
                        printf( PURPLE "A pessoa com o id %d muda da Zona B para a %s. \n" STOP, people.id, newZona);
                        free(newZona);
                    }
                    if (people.zona == PADARIA) {
                        sem_post(&semaforoPadaria);
                        newZona = defineNovaZona(&people);
                        printf( PURPLE "A pessoa com o id %d muda da Padaria para a %s. \n" STOP, people.id, newZona);
                        free(newZona);
                    }
                }
                else {
                    printf("A pessoa com o id %d nao quer mudar de zona \n", people.id);
                    if (probabilidade(config.probEntrarComa) == 1) { 
                        if (people.zona == ZONA_A) {
                            sem_post(&semaforoZonaA);
                        }
                        if (people.zona == ZONA_B) {
                            sem_post(&semaforoZonaB);
                        }
                        if (people.zona == PADARIA) {
                            sem_post(&semaforoPadaria);
                        }
                        people.estado == COMA;
                        printf( RED "A pessoa com o id %d bebeu demasiado e entrou em coma. \n" STOP, people.id);
                        enviaInformacao(sockfd, people.id, timestamp, 3, 1, people.zona);
                        printf( BLUE "A pessoa com o id %d está a levar soro para curar do coma alcoolico. \n" STOP, people.id);
                        sleep(config.tempoLevarSoro);

                        if (probabilidade(config.probMorrerComa) == 1){
                            if (people.zona == ZONA_A) {
                                sem_post(&semaforoZonaA);
                            }
                            if (people.zona == ZONA_B) {
                                sem_post(&semaforoZonaB);
                            }
                            if (people.zona == PADARIA) {
                                sem_post(&semaforoPadaria);
                            }
                            printf( WHITE "A pessoa com o id %d não aguentou e infelizmente morreu. \n" STOP, people.id);
                            enviaInformacao(sockfd, people.id, timestamp, 6, 1, people.zona);
                            break;
                        }
                        else {
                            if (people.zona == ZONA_A) {
                                sem_post(&semaforoZonaA);
                            }
                            if (people.zona == ZONA_B) {
                                sem_post(&semaforoZonaB);
                            }
                            if (people.zona == PADARIA) {
                                sem_post(&semaforoPadaria);
                            }
                            printf( WHITE "A pessoa com o id %d felizmente aguentou o tratamento e sobreviveu. \n" STOP, people.id);
                            enviaInformacao(sockfd, people.id, timestamp, 5, 1, people.zona);
                            break;
                        }     
                    }
                    else if (probabilidade(config.probEntrarComa) == 0) {
                        if (people.zona == ZONA_A) {
                            sem_post(&semaforoZonaA);
                        }
                        if (people.zona == ZONA_B) {
                            sem_post(&semaforoZonaB);
                        }
                        if (people.zona == PADARIA) {
                            sem_post(&semaforoPadaria);
                        }
                        printf(PURPLE "A pessoa com o id %d cansou-se e saiu da discoteca. \n" STOP, people.id);
                        people.desistiu == TRUE;
                        enviaInformacao(sockfd, people.id, timestamp, 1, 3, people.zona);
                    } 
                }
            }
        } 
        else {
            break;
        }    
    }
}




int readConfiguracao(char ficheiro[]) {     //funcao para ler a configuracao

    FILE* fp;
    fp = fopen(ficheiro, "r");    //abre o ficheiro para ler

    if(fp != NULL){
        // Obtem o tamanho do ficheiro:
        fseek(fp, 0, SEEK_END);
        long tamFicheiro = ftell(fp);
        rewind(fp);
        char buffer[tamFicheiro];
        fread(buffer, 1, tamFicheiro, fp);
        fclose(fp);

        int counter = 0;
        char *p = strtok(buffer, "\n");
        char *linhas[25];
        while (p != NULL) {
            linhas[counter++] = p;
            p = strtok(NULL, "\n");
        }
        char *array[2];
        char *val[17];
        for (int index = 0; index < 17; index++) {
            char *aux = strtok(linhas[index], ":");
            counter = 0;
            while (aux != NULL) {
                array[counter++] = aux;
                aux = strtok(NULL, ":");
            }
            val[index] = array[1];
        }
        
        config.tamanhoDiscoteca = (int)strtol(val[0], NULL, 10);
        config.tempoLimiteSimulacao = strtol(val[1], NULL, 10);
        config.tamanhoMaxFila1 = strtol(val[2], NULL, 10);
        config.tamanhoMaxFila2 = strtol(val[3], NULL, 10);
        config.tamanhoMaxZonaA = strtol(val[4], NULL, 10);
        config.tamanhoMaxZonaB = strtol(val[5], NULL, 10);
        config.tamanhoMaxPadaria = strtol(val[6], NULL, 10);
        config.tempoMedioChegada = strtol(val[7], NULL, 10);
        config.tempoMedioEspera = strtol(val[8], NULL, 10);
        config.tempoMaxEspera = strtol(val[9], NULL, 10);
        config.tempoLevarSoro = strtol(val[10], NULL, 10);
        config.probSerVIP = strtof(val[11], NULL);
        config.probDesistir = strtof(val[12], NULL);
        config.probEntrarComa = strtof(val[13], NULL);
        config.probMorrerComa = strtof(val[14], NULL);
        config.probIrComer = strtof(val[15], NULL);
        config.probMudarZona = strtof(val[16], NULL);
        
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
            pthread_mutex_unlock(&mutexVariaveisSimulacao);
            if (pthread_create(&tasksID[pessoaID], NULL, Pessoa, NULL)) {
                printf("Erro na criação da tarefa\n");
                exit(1);
            }
            pthread_mutex_lock(&mutexVariaveisSimulacao);          
        }
        usleep(10000);
        if (config.tempoLimiteSimulacao == tempoDecorrido) {
            enviaInformacao(sockfd, 0, 0, 99, 0, 0); 
        }
    }
    
}



void semaforosTrincos() {       
    if (pthread_mutex_init(&mutexVariaveisSimulacao, NULL) != 0) {
        printf("Inicializacao do trinco falhou.\n");
    }
    if (pthread_mutex_init(&mutexCriarPessoa, NULL) != 0) {
        printf("Inicializacao do trinco falhou.\n");
    }
    if (pthread_mutex_init(&mutexFilaDeEspera, NULL) != 0) {
        printf("Inicializacao do trinco falhou.\n");
    }
    sem_init(&semaforoFila1, 0, config.tamanhoMaxFila1);
    sem_init(&semaforoFila2, 0, config.tamanhoMaxFila2);
    sem_init(&semaforoDiscoteca, 0, config.tamanhoDiscoteca);
    sem_init(&semaforoZonaA, 0, config.tamanhoMaxZonaA);
    sem_init(&semaforoZonaB, 0, config.tamanhoMaxZonaB);
    sem_init(&semaforoPadaria, 0, config.tamanhoMaxPadaria);
    sem_init(&semaforoEnviaInformacao, 0, 1);
}


int main(int argc, char const * argv[]){

    sockfd = criaSocket();
    simulacao(argv[1]);
    close(sockfd);
    return 0;
}


