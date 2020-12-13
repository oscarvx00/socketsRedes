#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include "serverUtils.h"
#include "cola.h"

void commandIn(char *bf){

	static char lastCommand[32];

	//SE DEBE SPLITEAR PREVIAMENTE
	char bfCopy[512];
	strcpy(bfCopy, bf);
	
	Cola c = splitCommand(bf);
	
	char* command = colaSuprime(&c);
	
	if(strcmp(/*inputCommand result of split*/command, "LIST") == 0){
		commandList(bf);
	} else if(strcmp(command, "NEWGROUPS") == 0){
	 	//comando newgroups
	} else if(strcmp(command, "NEWNEWS") == 0){
		//comando NEWNEWS
	} else if(strcmp(command, "GROUP") == 0){
		//comando GROUP
	} else if(strcmp(command, "ARTICLE") == 0){
		//comando ARTICLE
	} else if(strcmp(command, "HEAD") == 0){
		//comando HEAD
	} else if(strcmp(command, "BODY") == 0){
		//comando BODY
	} else if(strcmp(command, "POST") == 0){
		//comando POST
	} else if(strcmp(command, "QUIT") == 0){
		//comando QUIT
	} else{
		//No reconocido
	}

	//strcpy(bf, "SALGO");

}


Cola splitCommand(char *bf){

	Cola c;
	
	if(colaCreaVacia(&c) != 0){
		printf("\033[0;31m");
		printf("ERROR CREANDO COLA");
		printf("\033[0m");
		//exit(-1);
	}

	char* token = strtok(bf, " ");

	while(token != NULL){
	//printf("\n%s", token);
		colaInserta(&c, token);
		token = strtok(NULL, " ");
	}

	
	
	return c;
}

void commandList(char *buf){

	//Devolver nombre de grupo, num de articulos, num del primero y num del ultimo.
	DIR* d;
	struct dirent *dir;
	d = opendir("./articulos");
	if(d){
		while((dir = readdir(d)) != NULL){
			printf("%s", dir->d_name);
			sprintf(buf, "\n%s", dir->d_name);
		}
		closedir(d);
	}
}

void commandNewgroups(Cola c){

	char *date = colaSuprime(&c);
	char *time = colaSuprime(&c);

	//Consulta los grupos disponibles desde la hora indicada

}

void commandNewnews(Cola c){

	char *groupName = colaSuprime(&c);
	char *date = colaSuprime(&c);
	char *time = colaSuprime(&c);

	//Consulta articulos disponibles del grupo a partir del dia y hora;

}

void commandGroup(Cola c){

	char *groupName = colaSuprime(&c);

	//Consulta los articulos del grupo, devolviendo numero del primero y del ultimo

}

void commandArticle(Cola c){

	char *articleNum = colaSuprime(&c);

	//Consulta el articulo
}

void commandHead(Cola c){

	char *articleNum = colaSuprime(&c);

	//Devuelve la cabecera del articulo
}

void commandBody(Cola c){

	char *articleNum = colaSuprime(&c);

	//Devuelve el cuerpo del articulo
}
	
