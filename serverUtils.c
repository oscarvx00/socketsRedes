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

void commandIn(int sockfd, char *bf, size_t len, int flag, char* hostName){

	static char lastCommand[32];

	sockfdGlobal = sockfd;
	bfGlobal = bf;
	lenGlobal = len;
	flagGlobal = flag;
	hostNameGlobal = hostName;

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
		sendMsg("500 Comando no conocido");
	}

	strcpy(bf, "SALGO");

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
	sendMsg("215 listado de los grupos en formato “nombre ultimo primero fecha descipcion”");

	DIR* d;
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
		sendMsg(".");
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
	
