//Grupo:
//2079120 Miguel Costa
//2105319 Ines Jardim

#include "config.h"

int main(int argc, char *argv[]) 
{
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

	printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
	printf("1 - Ligar Discoteca               \n");
	printf("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");
	int opcao = 0;
	while (opcao != 1) {
        printf("Opcao: ");
        scanf("%d", &opcao); // Le valor introduzido
		if (opcao != 1){
			printf("Opcao invalida.");
			opcao = 0;
		}
	}

}
