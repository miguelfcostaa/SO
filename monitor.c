//2079120 Miguel Costa
//2105319 Ines Jardim

#include "config.h"

int fimSimulacao = FALSE;

int numeroPessoas = 0, numeroDiasPassados = 1, tempoMedioEspera = 0, tamanhoFilaCentro1 = 0, numeroPessoasEmIsolamento = 0, tamanhoFilaCentro2 = 0, casosPositivosTotal = 0, casosPositivosAtivos = 0, casosEmEstudo = 0,
    numeroMortos = 0, casosRecuperados = 0, doentesNoHospital = 0, medicosDisponiveis = 0, numeroPessoasTestadasNosCentros = 0, somaTemposEspera = 0;


void escrever(sockfd){
	char String[100];
	FILE *fp;

	fp = fopen("feedback.txt","a");
	if (fp == NULL){
		printf("Ocorreu um erro ao abrir o ficheiro\n");
	}
	else {
		printf("Ficheiro carregado com sucesso\n");
	}
	
	strcpy(String,"Linha de teste");
	fputs(String, fp);
	fclose(fp);

}


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
    while (!fimSimulacao) {
        numero = read(sockfd, buffer, MAXLINE); // Le a mensagem do socket e guarda no buffer
        if (numero == 0) {            // Quando chega ao fim
            printf("FIM");
            break;
        } 
		else if (numero < 0) {
            printf("erro: nao foi possivel ler o socket. \n");
        } 
		else {
            printf("Mensagem Recebida");
            trataMensagem(buffer);
        }
    }
}

void trataMensagem(char mensagem[]) {
    // Auxiliario
    char bufferAuxiliario[MAXLINE];
    strcpy(bufferAuxiliario, mensagem);
    char mensagensSeparadas[15][30];
    char valoresSeparados[3][30];
    int index = 0;
    int indexMensagem = 0;
    int i = 0;
    int numeroMensagens = 0;
    char *p = strtok(bufferAuxiliario, "/");

    while (p != NULL) {
        strcpy(mensagensSeparadas[numeroMensagens++], p);
        p = strtok(NULL, "/");
    }

    // Obtem o head da lista ligada que se obtem separando bufferAuxiliario por
    // "_"
    while (indexMensagem < numeroMensagens) {
        index = 0;
        char *auxiliario = strtok(mensagensSeparadas[indexMensagem], "_");
        indexMensagem++;

        // Ciclo que percorre e vai separando pelos - e copiando para
        // valoresSeparados[i]
        while (auxiliario != NULL) {
            strcpy(valoresSeparados[index++], auxiliario);
            auxiliario = strtok(NULL, "_");
        }

        // Onde vai guardar os valores depois da divisao
        int acontecimento = strtol(valoresSeparados[1], NULL, 10);
        if (!strcmp(valoresSeparados[0], "Z") && !strcmp(valoresSeparados[2], "Z")) {
            if (acontecimento ==1) {
                fimSimulacao = TRUE;
            }
        } else {
            int numero;
            if (strcmp(valoresSeparados[0], "Z")) {
                numero = strtol(valoresSeparados[0], NULL, 10);
            } else {
                numero = -1;
            }
            int especificacaoAcontecimento;
            if (strcmp(valoresSeparados[2], "Z")) {
                especificacaoAcontecimento = strtol(valoresSeparados[2], NULL, 10);
            } else {
                especificacaoAcontecimento = -1;
            }
            switch (acontecimento) {
            case 0: // Pessoa chegou à fila de um centro.
                // numeroPessoas++;
                if (especificacaoAcontecimento == 1) {
                    tamanhoFilaCentro1 = numero;
                } else {
                    tamanhoFilaCentro2 = numero;
                }
                break;
            case 1:
                casosPositivosAtivos = numero;
                break;
            case 2:
                casosPositivosTotal = numero;
                // printf("%s\n",mensagensSeparadas[indexMensagem]);
                break;
            case 3:
                casosEmEstudo = numero;
                break;
            case 4:
                numeroPessoasEmIsolamento = numero;
                break;
            case 5:
                numeroPessoas = numero;
                break;
            case 6:
                doentesNoHospital = numero;
                break;
            case 7:
                tempoMedioEspera = numero;
                break;
            case 8:
                casosRecuperados = numero;
                break;
            case 9:
                numeroMortos = numero;
                break;
            case 10: // Passou um dia na simulação
                numeroDiasPassados++;
                break;
            case 11: // Medico criado
                medicosDisponiveis = numero;
                break;
            }
        }
    }
    imprimirInformacao();
}

int main(int argc, char *argv[]) {
	

	printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
	printf("1 - Ligar Discoteca               \n");
	printf("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");
	int opcao = 0;
	while (opcao != 1) {
        printf("Opcao: ");
        scanf("%d", &opcao); // Le valor introduzido
		switch (opcao)
		{
		case 0:
			printf("erro: opcao invalida");
			break;
		case 1:
			socketservidor();
			break;
		default:
			break;
		}
	}
}
