#include "config.h"

//Variaveis globais
int sockfd = 0; // inicia socket
int pessoaID = 0;
int tempoDecorrido = 0; // tempo decorrido em segundos
int minutosPassados = 0;

//configuracao e filas
struct configuracao config;
struct Fila1 filas1;
struct Fila2 filas2;


//Trincos e Semaforos
pthread_mutex_t mutexCriarPessoa;
pthread_mutex_t mutexFilaDeEspera;
pthread_mutex_t mutexVariaveisSimulacao;
sem_t semaforoEnviaInformacao;
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
float probSerVIP = 0;
float probDesistir = 0;
float probMorrerComa = 0;

//TAREFAS
pthread_t tasksID[SIZE_TASKS]; // pessoas e médicos
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
    sleep(1);
    sem_post(&semaforoEnviaInformacao);
}


//<<<<<<<<<<<<<<<<<<<<<<<< F.AUX >>>>>>>>>>>>>>>>>>>>>>>>>>>>>

int probabilidade (float valor) {
	return (randomNumber(100, 0)) > (valor * 100);
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

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

struct pessoa criaPessoa() {

    pthread_mutex_lock(&mutexCriarPessoa);
    
    struct pessoa people;

    people.id = pessoaID;
    people.sexualidade = randomNumber(HOMEM, MULHER);
    people.fila = randomNumber(2, 1);
    //people.fila = 1;
    people.zona = randomNumber(2, 0);
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
    people.resultadoTeste = NAO_FEZ_TESTE;
    people.tempoMaxEsperaP = randomNumber(config.tempoMaxEspera, (config.tempoMaxEspera * 2) / 3);
    /*
    printf("Criado Pessoa %d: \n", pessoaID);
    printf("id: %d\n",people.id);
    printf("sexualidade: %d\n",people.sexualidade);
    printf("fila: %d\n",people.fila);
    printf("zona: %d\n",people.zona);
    printf("vip: %d\n",people.vip);
    printf("desistiu: %d\n",people.desistiu);
    printf("pessoas a frente: %d\n",people.nPessoasAFrenteDesistir);
    printf("estado: %d\n",people.estado);
    printf("tempo: %d\n",people.tempoMaxEsperaP);
    */
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

    if (people->fila == 1) {
        pthread_mutex_lock(&mutexFilaDeEspera);
        int nPessoasNaFila1 = filas1.nPessoasEspera;
        pthread_mutex_unlock(&mutexFilaDeEspera);

        if (nPessoasNaFila1 < config.tamanhoMaxFila1) // Se o numero de pessoas na fila de espera for menor que o tamanho da fila avança
        {
            tipoDePessoa = defineTipoPessoa(people);
            printf( CYAN "%s chegou a fila 1, as %d \n" STOP, tipoDePessoa, timestamp);
            free(tipoDePessoa);
            enviaInformacao(sockfd, people->id, timestamp, 0, 1, people->zona);  
            /*
            printf("n pessoas a frente para desistir: %d \n", people->nPessoasAFrenteDesistir);
            printf("n pessoas na fila: %d \n", nPessoasNaFila1); 
            */
            if (people->nPessoasAFrenteDesistir < nPessoasNaFila1) //O numero de pessoas na fila é superior ao numero que a pessoa admite ter a sua frente, entao ela desiste
            {
                pthread_mutex_lock(&mutexFilaDeEspera);
                filas1.nPessoasEspera--;
                pthread_mutex_unlock(&mutexFilaDeEspera);
                tipoDePessoa = defineTipoPessoa(people);
                printf(YELLOW "%s desistiu da fila do 1 porque tinha muita gente a frente.\n" STOP, tipoDePessoa);
                free(tipoDePessoa);
                enviaInformacao(sockfd, people->id, timestamp, 1, 1, people->zona);   
                people->desistiu = TRUE;
            }
            else 
            {
                pthread_mutex_lock(&mutexFilaDeEspera);
                filas1.nPessoasEspera++;
                pthread_mutex_unlock(&mutexFilaDeEspera);
                pthread_mutex_lock(&mutexVariaveisSimulacao);
                waitTime1 = (randomNumber((config.tempoMedioChegada * 2), (config.tempoMedioEspera)));
                pthread_mutex_unlock(&mutexVariaveisSimulacao);
                /*
                printf("tempo de espera: %d \n", waitTime1);     
                printf("tempo maximo de espera: %d \n", people->tempoMaxEsperaP); 
                */
                if (waitTime1 > people->tempoMaxEsperaP) //se tempo que a pessoa vai esperar é maior que o tempo maximo que ele deseja esperar, esta desiste
                {
                    people->desistiu = TRUE;
                    pthread_mutex_lock(&mutexFilaDeEspera);
                    filas1.nPessoasEspera--;
                    pthread_mutex_unlock(&mutexFilaDeEspera);
                    tipoDePessoa = defineTipoPessoa(people);
                    printf( YELLOW "%s desistiu da fila do 1 porque esteve muito tempo a espera.\n" STOP, tipoDePessoa);
                    free(tipoDePessoa);
                    enviaInformacao(sockfd, people->id, timestamp, 1, 1, people->zona); 
                }
                else //senao ela entra na discoteca.
                {
                    pthread_mutex_lock(&mutexFilaDeEspera);
                    filas1.nPessoasEspera--;
                    pthread_mutex_unlock(&mutexFilaDeEspera);
                    tipoDePessoa = defineTipoPessoa(people);
                    zonaP = defineZona(people);
                    printf(GREEN "%s entrou na discoteca e %s \n" STOP, tipoDePessoa, zonaP);
                    free(tipoDePessoa);
                    free(zonaP);
                    enviaInformacao(sockfd, people->id, timestamp, 2, 1, people->zona);
                    if (probabilidade(config.probEntrarComa) == 1) {
                        people->estado == COMA;
                        tipoDePessoa = defineTipoPessoa(people);
                        printf(RED "%s bebeu demasiado e entrou em coma. \n" STOP, tipoDePessoa);
                        free(tipoDePessoa);
                        enviaInformacao(sockfd, people->id, timestamp, 3, 1, people->zona);
                    } 
                    else {
                        tipoDePessoa = defineTipoPessoa(people);
                        printf(PURPLE "%s cansou-se e saiu da discoteca. \n" STOP, tipoDePessoa);
                        free(tipoDePessoa);
                        enviaInformacao(sockfd, people->id, timestamp, 1, 1, people->zona);
                    } 
                }
            }
        }
        else {
            pthread_mutex_unlock(&mutexFilaDeEspera);
            people->desistiu = TRUE; 
        }
    }
    else if (people->fila == 2) {
        pthread_mutex_lock(&mutexFilaDeEspera);
        int nPessoasNaFila2 = filas2.nPessoasNormalEspera;
        pthread_mutex_unlock(&mutexFilaDeEspera);

        if (nPessoasNaFila2 < config.tamanhoMaxFila2) // Se o numero de pessoas na fila de espera for menor que o tamanho da fila avança
        {
            tipoDePessoa = defineTipoPessoa(people);
            printf( CYAN "%s chegou a fila 2, as %d \n" STOP, tipoDePessoa, timestamp);
            free(tipoDePessoa);
            enviaInformacao(sockfd, people->id, timestamp, 0, 2, people->zona);  
            /*
            printf("n pessoas a frente para desistir: %d \n", people->nPessoasAFrenteDesistir);
            printf("n pessoas na fila: %d \n", nPessoasNaFila2); 
            */
            if (people->nPessoasAFrenteDesistir < nPessoasNaFila2) //O numero de pessoas na fila é superior ao numero que a pessoa admite ter a sua frente, entao ela desiste
            {
                pthread_mutex_lock(&mutexFilaDeEspera);
                filas2.nPessoasNormalEspera--;
                pthread_mutex_unlock(&mutexFilaDeEspera);
                tipoDePessoa = defineTipoPessoa(people);
                printf(YELLOW "%s desistiu da fila do 2 porque tinha muita gente a frente.\n" STOP, tipoDePessoa);
                free(tipoDePessoa);
                enviaInformacao(sockfd, people->id, timestamp, 2, 2, people->zona);   
                people->desistiu = TRUE;
            }
            else 
            {
                pthread_mutex_lock(&mutexFilaDeEspera);
                filas2.nPessoasNormalEspera++;
                pthread_mutex_unlock(&mutexFilaDeEspera);
                pthread_mutex_lock(&mutexVariaveisSimulacao);
                waitTime2 = (randomNumber((config.tempoMedioChegada * 2), (config.tempoMedioEspera)));
                pthread_mutex_unlock(&mutexVariaveisSimulacao);
                /*
                printf("tempo de espera: %d \n", waitTime2);     
                printf("tempo maximo de espera: %d \n", people->tempoMaxEsperaP); 
                */
                if (waitTime2 > people->tempoMaxEsperaP) //se tempo que a pessoa vai esperar é maior que o tempo maximo que ele deseja esperar, esta desiste
                {
                    people->desistiu = TRUE;
                    pthread_mutex_lock(&mutexFilaDeEspera);
                    filas2.nPessoasNormalEspera--;
                    pthread_mutex_unlock(&mutexFilaDeEspera);
                    tipoDePessoa = defineTipoPessoa(people);
                    printf(YELLOW "%s desistiu da fila do 2 porque esteve muito tempo a espera.\n" STOP, tipoDePessoa);
                    free(tipoDePessoa);
                    enviaInformacao(sockfd, people->id, timestamp, 1, 2, people->zona); 
                }
                else //senao ela entra na discoteca.
                {
                    pthread_mutex_lock(&mutexFilaDeEspera);
                    filas2.nPessoasNormalEspera--;
                    pthread_mutex_unlock(&mutexFilaDeEspera);
                    tipoDePessoa = defineTipoPessoa(people);
                    zonaP = defineZona(people);
                    printf(GREEN "%s entrou na discoteca e %s \n" STOP, tipoDePessoa, zonaP);
                    free(tipoDePessoa);
                    free(zonaP);
                    enviaInformacao(sockfd, people->id, timestamp, 2, 2, people->zona);
                    if (probabilidade(config.probEntrarComa) == 1) {
                        people->estado == COMA;
                        tipoDePessoa = defineTipoPessoa(people);
                        printf(RED "%s bebeu demasiado e entrou em coma. \n" STOP, tipoDePessoa);
                        free(tipoDePessoa);
                        enviaInformacao(sockfd, people->id, timestamp, 3, 2, people->zona);
                    } 
                    else {
                        tipoDePessoa = defineTipoPessoa(people);
                        printf(PURPLE "%s cansou-se e saiu da discoteca. \n" STOP, tipoDePessoa);
                        free(tipoDePessoa);
                        enviaInformacao(sockfd, people->id, timestamp, 1, 2, people->zona);
                    }
                }
            }
        }
        else {
            pthread_mutex_unlock(&mutexFilaDeEspera);
            people->desistiu = TRUE; 
        }
    }
}


void Pessoa(void *ptr) {
    struct pessoa people = criaPessoa();
    pessoasCriadas[pessoa.id] = &people;

    while (TRUE) {
        FilaDeEspera(&people);
        if (people.desistiu == TRUE) {
            // sprintf(mensagem, "%d-%d-%d-%d", pessoa.id, "Z", 3, "Z");
            // enviarInformacao(mensagem);
            //int tipoTeste = -1;
            //int tempoEsperaTeste = 0;
            // printf("TEMPO ESPERA TESTE em ms: %d\n",tempoEsperaTeste);
            //usleep(tempoEsperaTeste);
            //fazerTeste(&pessoa);

            /*if (people.estado == COMA) {

            }

            if (pessoa.estadoTeste == POSITIVO) {
                printf(YELLOW "Pessoa %d testou positivo \n" STOP, pessoa.id);
                sprintf(mensagem, "%d-%d-%d-%d", pessoa.id, "Z", 8, "Z");
                enviarInformacao(mensagem);
                sem_init(&pessoa.semaforoPessoa, 0, 0);
                pessoa.numeroDiasDesdePositivo = 0;
                pessoa.estado = ISOLAMENTO;
                break;
            } 
            else if (pessoa.estadoTeste == NEGATIVO) {
                printf(YELLOW "Pessoa %d testou negativo \n" STOP, pessoa.id);
                sprintf(mensagem, "%d-%d-%d-%d", pessoa.id, "Z", 9, 0);
                enviarInformacao(mensagem);
                pessoa.estado = SOBREVIVEU;
                break;
            } 
            else {
                printf(YELLOW "Pessoa %d testou inconclusivo \n" STOP, pessoa.id);
                sprintf(mensagem, "%d-%d-%d-%d", pessoa.id, "Z", 12, "Z");
                enviarInformacao(mensagem);
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
        printf(CYAN
                "A pessoa com id %d foi transportada para o hospital.\n" STOP,
                pessoa.id);
        sprintf(mensagem, "%d-%d-%d-%d", pessoa.id, "Z", 4, "Z");
        enviarInformacao(mensagem);
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

        int i = 0;
        char *p = strtok(buffer, "\n");
        char *linhas[25];
        while (p != NULL) {
            linhas[i++] = p;
            p = strtok(NULL, "\n");
        }
        char *array[2];
        char *values[16];
        for (int index = 0; index < 16; index++) {
            char *aux = strtok(linhas[index], ":");
            i = 0;
            while (aux != NULL) {
                array[i++] = aux;
                aux = strtok(NULL, ":");
            }
            values[index] = array[1];
        }
        
        config.tamanhoDiscoteca = (int)strtol(values[0], NULL, 10);
        config.tempoLimiteSimulacao = strtol(values[1], NULL, 10);
        config.tamanhoMaxFila1 = strtol(values[2], NULL, 10);
        config.tamanhoMaxFila2 = strtol(values[3], NULL, 10);
        config.tamanhoMaxZonaA = strtol(values[4], NULL, 10);
        config.tamanhoMaxZonaB = strtol(values[5], NULL, 10);
        config.tamanhoMaxPadaria = strtol(values[6], NULL, 10);
        config.tempoMedioChegada = strtol(values[7], NULL, 10);
        config.tempoMedioEspera = strtol(values[8], NULL, 10);
        config.tempoMaxEspera = strtol(values[9], NULL, 10);
        config.tempoFazerTeste = strtol(values[10], NULL, 10);
        config.probSerVIP = strtof(values[11], NULL);
        config.probDesistir = strtof(values[12], NULL);
        config.probEntrarComa = strtof(values[13], NULL);
        config.probMorrerComa = strtof(values[14], NULL);
        
        
        /*
        printf( "Tamanho Discoteca: %d \n", config.tamanhoDiscoteca);	
        printf("Tempo Limite Simulacao: %d \n", config.tempoLimiteSimulacao);	
        printf( "Tamanho Max Fila 1: %d \n", config.tamanhoMaxFila1);	
        printf( "Tamanho Max Fila 2: %d \n", config.tamanhoMaxFila2);	
        printf( "Tamanho Max Zona A: %d \n", config.tamanhoMaxZonaA);	
        printf( "Tamanho Max Zona B: %d \n", config.tamanhoMaxZonaB);	
        printf( "Tamanho Max Padaria: %d \n", config.tamanhoMaxPadaria);
        printf( "Prob ser vip: %f \n", config.probSerVIP);
        printf( "Prob desistir: %f \n", config.probDesistir);
        printf( "Prob entrar em coma: %f \n", config.probEntrarComa);
        */
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
            sleep(1);
        }
    }
    if (config.tempoLimiteSimulacao == tempoDecorrido) {
        enviaInformacao(sockfd, 0, 0, 99, 0, 0); 
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

    sem_init(&filas1.filaEspera, 0, 0);
    sem_init(&semaforoEnviaInformacao, 0, 1);
    sem_init(&filas2.filaEsperaPrioritaria, 0, 0);
    sem_init(&filas2.filaEsperaNormal, 0, 0);
}


int main(int argc, char const * argv[]){

    sockfd = criaSocket();
    simulacao(argv[1]);
    close(sockfd);
    return 0;
}


