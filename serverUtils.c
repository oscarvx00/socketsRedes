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
	
	
	Cola c = splitLine(bf, " ");
	
	char* command = colaSuprime(&c);
	
	if(strcmp(/*inputCommand result of split*/command, "LIST") == 0){
		commandList(bf);
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
		if(!colaVacia(&c)){
			char *str = colaSuprime(&c);
			commandHead(str);
		} else{
			sendMsg("501 Error de sintaxis en HEAD numArticle");
		}
	} else if(strcmp(command, "BODY") == 0){
		if(!colaVacia(&c)){
			char *str = colaSuprime(&c);
			commandBody(str);
		} else{
			sendMsg("501 Error de sintaxis en BODY numArticle");
		}
	} else if(strcmp(command, "NEWGROUPS") == 0){
		char *date, *time;
		if(!colaVacia(&c)){
			date = colaSuprime(&c);
			if(!colaVacia(&c)){
				time = colaSuprime(&c);
				commandNewGroups(date, time);
			} else{
				sendMsg("501 Error de sintaxis en NEWGROUPS date(aaaammdd) time(hhmmss)");
			}
		} else{
			sendMsg("501 Error de sintaxis en NEWGROUPS date(aaaammdd) time(hhmmss)");
		}
	} else if(strcmp(command, "NEWNEWS") == 0){
		char *location, *date, *time;
		if(!colaVacia(&c)){
			location = colaSuprime(&c);
			if(!colaVacia(&c)){
				date = colaSuprime(&c);
				if(!colaVacia(&c)){
					time = colaSuprime(&c);
					commandNewNews(location, date, time);
				}else{
				sendMsg("501 Error de sintaxis en NEWNEWS newsgroup date(aaaammdd) time(hhmmss)");
				}
			}
			else{
				sendMsg("501 Error de sintaxis en NEWNEWS newsgroup date(aaaammdd) time(hhmmss)");
			}
		} else{
			sendMsg("501 Error de sintaxis en NEWNEWS newsgroup date(aaaammdd) time(hhmmss)");
		}
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

Cola splitLine(char *bf, char *character){

	Cola c;
	
	char bfCopy[512];
	strcpy(bfCopy, bf);

	//printf("BUFFER: %s\nCOPY: %s\nCOMP: %d", bf, bfCopy, strcmp(bf, bfCopy));

	if(colaCreaVacia(&c) != 0){
		printf("\033[0;31m");
		printf("ERROR CREANDO COLA COMMAND");
		printf("\033[0m");
		exit(-1);
	}
	colaCreaVacia(&c);

	char* token = strtok(bfCopy, character);

	while(token != NULL){
	//printf("\n%s", token);
		colaInserta(&c, token);
		token = strtok(NULL, character);
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


void commandGroup(char* str){

	//Consulta los articulos del grupo, devolviendo numero de articulos, numero del primero y del ultimo
	char *groupInput = malloc(strlen(str));
	strcpy(groupInput, str);

	Cola c = splitLine(str, ".");
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

	//printf("LOCATION: %s", location);

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
		sprintf(msg, "%d %d %010d %010d %s", 211, count, first, last, groupInput);

		sendMsg(msg);
	} else{
		sendMsg("501 Error de sintaxis en GROUP newsgroup");
	}

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
						fclose(f);
					}
				}
			}
		}
	}
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

void commandHead(char *str){

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
						int flag = 1;
						while(fgets(buff, lenGlobal, f) && flag){
							
							if(!strcmp(buff, "\r\n") || !strcmp(buff, "\n")){
								flag = 0;
							} else{
								sendMsg(buff);
							}

						}
						fclose(f);
					}
				}
			}
		}
	}
}

void commandBody(char *str){

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
						int bodyReached = 0;
						while(fgets(buff, lenGlobal, f)){							
							if(!strcmp(buff, "\r\n") || !strcmp(buff, "\n")){
								bodyReached = 1;
							} else if(bodyReached){
								sendMsg(buff);
							}
						}
						fclose(f);
					}
				}
			}
		}
	}
}


void commandNewGroups(char *date, char *time){

	FILE *f;

	struct grupo gStruc;

	int d, t;
	d = atoi(date);
	t = atoi(time);

	sendMsg("231 Nuevos grupos");

	if((f = fopen("grupos", "r")) == NULL){
		perror("Error abriendo archivo");
	} else{
		while(fscanf(f, "%s %d %d %d %d %[^\n]", gStruc.loc, &gStruc.last, &gStruc.first, &gStruc.date, &gStruc.time, gStruc.descr) != EOF){
			if(gStruc.date > d && gStruc.time >= t)
				sendMsg(gStruc.loc);
		}
	}
	sendMsg(".");

}


void commandNewNews(char *loc, char *date, char *time){


	char buff[lenGlobal];

	char *groupInput = malloc(strlen(loc));
	strcpy(groupInput, loc);

	Cola c = splitLine(loc, ".");
	char fileStart[] = "./articulos/";
	char *location;



	int dateInt = atoi(date);
	int timeInt = atoi(time);

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

	//printf("LOCATION: %s", location);

	
	DIR *d;
	FILE *f;
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
				
				char *path;
				if((path = malloc(strlen(location) + 4 + strlen(dir->d_name))) == NULL){
						fprintf(stderr, "ERROR MALLOC");
						exit(-1);
				}
					
				strcpy(path, location);
				strcat(path, "/");
				strcat(path, dir->d_name);				

				if((f = fopen(path, "r")) == NULL){
						perror("Error abriendo archivo");
				} 
				else{
					
					//fseek(f, 0, SEEK_SET);
					int rowCount = 0;
					char *d, *t;
					char *subject, *id;
					while(fgets(buff, lenGlobal, f) && rowCount < 4 /*Leemos la cabecera*/){
						if(rowCount == 2){
							Cola cF = splitLine(buff, " ");
							int colaCount = 0;
							while(!colaVacia(&cF) && colaCount < 3){
								if(colaCount == 1){
									d = colaSuprime(&cF);
								} else if(colaCount == 2){
									t = colaSuprime(&cF);
								} else{
									colaSuprime(&cF);
								}
								colaCount++;
							}
						} else if(rowCount == 1){
							if((subject = malloc(strlen(buff) + 4)) == NULL){
								fprintf(stderr, "ERROR MALLOC");
								exit(-1);
							}
							strcpy(subject, buff);
						} else if(rowCount == 3){
							if((id = malloc(strlen(buff) + 4)) == NULL){
								fprintf(stderr, "ERROR MALLOC");
								exit(-1);
							}
							strcpy(id, buff);
						}
						rowCount++;

					}
					fclose(f);
					
					if(dateInt <= atoi(d) && timeInt <= atoi(t)){
						//Enviar MSG
						snprintf(buff, lenGlobal, "%s %s %s", dir->d_name, subject, id);
						//strcpy(buff, dir->d_name);
						//strcat(buff, " ");
						//strcpy(buff, subject);
						sendMsg(buff);
					}
					free(subject); free(id);
				}
				free(path);
			}
		}
		closedir(d);

	} else{
		sendMsg("501 Error de sintaxis en GROUP newsgroup");
	}
}
