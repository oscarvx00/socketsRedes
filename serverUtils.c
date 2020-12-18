#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include "serverUtils.h"
#include "cola.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <netdb.h>
#include <time.h>
#include <unistd.h>

int sockfdGlobal;
char* bfGlobal;
size_t lenGlobal;
int flagGlobal;
char* hostNameGlobal;

char *selectedGroupPath = NULL;

int commandIn(int sockfd, char *bf, size_t len, int flag, char* hostName){

	int serverFlag = 1;

	sockfdGlobal = sockfd;
	bfGlobal = bf;
	lenGlobal = len;
	flagGlobal = flag;
	hostNameGlobal = hostName;

	//SE DEBE SPLITEAR PREVIAMENTE

		/*printf("\033[0;31m");
		printf("COMMAND IN: %s", bf);
		printf("\033[0m");*/
	
	
	Cola c = splitCommand(bf);
	
	char* command = colaSuprime(&c);
	
	if(strcmp(/*inputCommand result of split*/command, "LIST") == 0){
		commandList(bf);
	} else if(strcmp(command, "NEWGROUPS") == 0){
	 	//comando newgroups
	} else if(strcmp(command, "NEWNEWS") == 0){
		//comando NEWNEWS
	} else if(strcmp(command, "GROUP") == 0){
		if(!colaVacia(&c)){
			char* str = colaSuprime(&c);
			commandGroup(str);
		} else{
			sendMsg("501 Error de sintaxis en GROUP newsgroup");
		}
	} else if(strcmp(command, "ARTICLE") == 0){
		if(!colaVacia(&c)){
			char *str = colaSuprime(&c);
			commandArticle(str);
		} else{
			sendMsg("501 Error de sintaxis en ARTICLE numArticle");
		}
	} else if(strcmp(command, "HEAD") == 0){
		//comando HEAD
	} else if(strcmp(command, "BODY") == 0){
		//comando BODY
	} else if(strcmp(command, "POST") == 0){
		//comando POST
	} else if(strcmp(command, "QUIT") == 0){
		sendMsg("205 Adios");
		serverFlag = 0;
	} else{
		sendMsg("500 Comando no conocido");
	}

	sendMsg("\r\n");

	return serverFlag;

}

void sendMsg(char* msg){
	//printf ("\033[32;1m %s \033[0m\n", msg);

	char *aux;

	if(!strcmp(hostNameGlobal, "NONAME")){
		printf("\n%s", msg);
		return;
	}

	if (send(sockfdGlobal, msg, lenGlobal, flagGlobal) != lenGlobal) erroutUtils(hostNameGlobal);
}

void erroutUtils(char *hostname)
{
	printf("Connection with %s aborted on error\n", hostNameGlobal);
	exit(1);     
}

Cola splitCommand(char *bf){

	Cola c;
	
	char bfCopy[512];
	strcpy(bfCopy, bf);

	printf("BUFFER: %s\nCOPY: %s\nCOMP: %d", bf, bfCopy, strcmp(bf, bfCopy));

	if(colaCreaVacia(&c) != 0){
		printf("\033[0;31m");
		printf("ERROR CREANDO COLA COMMAND");
		printf("\033[0m");
		exit(-1);
	}
	colaCreaVacia(&c);

	char* token = strtok(bfCopy, " ");

	while(token != NULL){
	//printf("\n%s", token);
		colaInserta(&c, token);
		token = strtok(NULL, " ");
	}

	return c;
}

void commandList(char *buf){

	//Devolver nombre de grupo, num de articulos, num del primero y num del ultimo.
	sendMsg("215 listado de los grupos en formato “nombre ultimo primero fecha descipcion”");

	FILE *f;

	char buff[lenGlobal];

	if((f = fopen("grupos", "r")) == NULL){
		perror("Error abriendo archivo");
	} else{
		while(fgets(buff, lenGlobal, f) != NULL){
			sendMsg(buff);
		}
	}
	sendMsg(".");
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
	sendMsg(".");
	//Consulta articulos disponibles del grupo a partir del dia y hora;

}

void commandGroup(char* str){

	//Consulta los articulos del grupo, devolviendo numero de articulos, numero del primero y del ultimo
	char *groupInput = malloc(strlen(str));
	strcpy(groupInput, str);

	Cola c = splitGroup(str);
	char fileStart[] = "./articulos/";
	char *location;

	char *tema, *subtema;
	if(!colaVacia(&c)){
		tema = colaSuprime(&c);
	} else{ 
		sendMsg("501 Error de sintaxis en GROUP newsgroup");
		return;
	}
	if(!colaVacia(&c)){
		subtema = colaSuprime(&c);
	} else{
		sendMsg("501 Error de sintaxis en GROUP newsgroup");
		return;
	}

	location = malloc(strlen(fileStart) + strlen(tema) + strlen(subtema) + 2);
	
	//sprintf(location, "%s/%s", tema, subtema);
	strcpy(location, fileStart);
	strcat(location, tema);
	strcat(location, "/");
	strcat(location, subtema);

	printf("LOCATION: %s", location);

	selectedGroupPath = malloc(strlen(location));
	strcpy(selectedGroupPath, location);

	
	DIR* d;
	struct dirent *dir;
	d = opendir(location);

	int first;
	int last;
	int count = 0;

	char *msg;
	if(d){
		while((dir = readdir(d)) != NULL){
			//printf("%s", dir->d_name);
			//sprintf(buf, "\n%s", dir->d_name);
			if(strcmp(".", dir->d_name) && strcmp("..", dir->d_name)){
				if(count == 0){
					first = atoi(dir->d_name);
				}
				last = atoi(dir->d_name);
				count++;
			}
		}
		closedir(d);

		msg = malloc(512);
		sprintf(msg, "%d %d %05d %05d %s", 211, count, first, last, groupInput);

		sendMsg(msg);
	} else{
		sendMsg("501 Error de sintaxis en GROUP newsgroup");
	}

}

Cola splitGroup(char *bf){

	Cola cola;

	char bfCopy[512];
	strcpy(bfCopy, bf);

	printf("BFCOPY %s", bfCopy);

	if(colaCreaVacia(&cola) != 0){
		printf("\033[0;31m");
		printf("ERROR CREANDO COLA GROUP");
		printf("\033[0m");
		exit(-1);
	}

	char* token = strtok(bfCopy, ".");

	while(token != NULL){
	//printf("\nTOKEEEN: %s", token);
		colaInserta(&cola, token);
		token = strtok(NULL, ".");
	}

	return cola;
}

void commandArticle(char *str){

	char *articleNumber;
	articleNumber = malloc(strlen(str) + 1);
	strcpy(articleNumber, str);

	char buff[lenGlobal];

	if(selectedGroupPath == NULL){
		sendMsg("500 No se ha seleccionado ningun grupo (GROUP)");
		return;
	} 

	DIR *d;
	struct dirent *dir;

	d = opendir(selectedGroupPath);

	char *path;

	if(d){
		while((dir = readdir(d)) != NULL){
			if(strcmp(".", dir->d_name) && strcmp("..", dir->d_name)){
				//printf("\n\tSUBFOLDER: %s", dir->d_name);

				if(!strcmp(articleNumber, dir->d_name)){
					char *path;
					if((path = malloc(strlen(selectedGroupPath) + 2 + strlen(dir->d_name))) == NULL){
							fprintf(stderr, "ERROR MALLOC");
							exit(-1);
					}
						
					strcpy(path, selectedGroupPath);
					strcat(path, "/");
					strcat(path, dir->d_name);

					//Lectura y envio del articulo
					char *initialMsg = malloc(300);
					strcpy(initialMsg, "223 ");
					strcat(initialMsg, articleNumber);
					strcat(initialMsg, " articulo recuperado");
					sendMsg(initialMsg);

					FILE *f;

					

					if((f = fopen(path, "r")) == NULL){
						perror("Error abriendo archivo");
					} else{
						fseek(f, 0, SEEK_SET);
						while(fgets(buff, lenGlobal, f)){
							
							if(!strcmp(buff, "\r\n") || !strcmp(buff, "\n")){
								strcpy(buff, " \n\0");
							}
							sendMsg(buff);

						}
						fclose(c);
					}
					//sendMsg("\r\n");
				}
			}
		}
	}

	//printf("\n\n\n\nARTICLE PATH: %s", path);
	//sendMsg(path);


}

/*char* findArticle(const char *folder, const char *articleNumber){

	DIR *d;
	struct dirent *dir;

	d = opendir(folder);

	printf("\nBUSCANDO EN: %s", folder);
	if(d){
		while((dir = readdir(d)) != NULL){
			if(strcmp(".", dir->d_name) && strcmp("..", dir->d_name)){
				printf("\n\tSUBFOLDER: %s", dir->d_name);

				char *path;
				if((path = malloc(strlen(folder) + 2 + strlen(dir->d_name))) == NULL){
						fprintf(stderr, "ERROR MALLOC");
						exit(-1);
				}
					
				strcpy(path, folder);
				strcat(path, "/");
				strcat(path, dir->d_name);

				if(!strcmp(articleNumber, dir->d_name)){
					return path;
				} else{
				
					char *aux = findArticle(path, articleNumber);
					printf("\n\t\tPATH: %s", path);
					if(aux != NULL)		return aux;

				}
			}
		}
	}
	return NULL;
}*/

void commandHead(Cola c){

	char *articleNum = colaSuprime(&c);

	//Devuelve la cabecera del articulo
}

void commandBody(Cola c){

	char *articleNum = colaSuprime(&c);

	//Devuelve el cuerpo del articulo
}
	
