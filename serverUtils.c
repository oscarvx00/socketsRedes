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
#include <string.h>
#include <time.h>
#include <unistd.h>

int sockfdGlobal;
char* bfGlobal;
size_t lenGlobal;
int flagGlobal;
char* hostNameGlobal;

int commandIn(int sockfd, char *bf, size_t len, int flag, char* hostName){

	int serverFlag = 1;

	static char lastCommand[32];

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
		//commandGroup(c);
	} else if(strcmp(command, "ARTICLE") == 0){
		//comando ARTICLE
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

	sendMsg(".");

	return serverFlag;

}

void sendMsg(char* msg){
	printf ("\033[32;1m %s \033[0m\n", msg);

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

	if(colaCreaVacia(&c) != 0){
		printf("\033[0;31m");
		printf("ERROR CREANDO COLA COMMAND");
		printf("\033[0m");
		exit(-1);
	}

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

	/*DIR* d;
	struct dirent *dir;
	d = opendir("./articulos");
	if(d){
		while((dir = readdir(d)) != NULL){
			//printf("%s", dir->d_name);
			//sprintf(buf, "\n%s", dir->d_name);
			if(strcmp(dir->d_name, ".") && strcmp(dir->d_name, "..")) //Si no es . ni .. enviamos.
				sendMsg(dir->d_name);
		}
		closedir(d);
	}*/

	FILE *f;

	char buff[lenGlobal];

	if((f = fopen("grupos", "r")) == NULL){
		perror("Error abriendo archivo");
	} else{
		while(fgets(buff, lenGlobal, f) != NULL){
			sendMsg(buff);
		}
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
sendMsg(".");
	//Consulta articulos disponibles del grupo a partir del dia y hora;

}

void commandGroup(char* str){

	//Consulta los articulos del grupo, devolviendo numero de articulos, numero del primero y del ultimo

	Cola c = splitGroup(str);
	char fileStart[] = "./";
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

	printf("SERVER LOC: %s",location);
	

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
	
