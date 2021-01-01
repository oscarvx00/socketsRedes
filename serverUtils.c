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

#include <sys/sem.h> 
#include <sys/wait.h> 
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include <sys/msg.h>
#include <limits.h>

int sockfdGlobal;
char* bfGlobal;
size_t lenGlobal;
int flagGlobal;
char* hostNameGlobal;

int socketMode = 2;

struct sockaddr_in clientaddr_inGlobal;
int addrlenGlobal;

char *selectedGroupPath = NULL;

int commandIn(int sockfd, char *bf, size_t len, int flag, char* hostName, int mode, struct sockaddr_in clientaddr_in, socklen_t addrlen){

	int serverFlag = 1;

	sockfdGlobal = sockfd;
	bfGlobal = bf;
	lenGlobal = len;
	flagGlobal = flag;
	hostNameGlobal = hostName;

	socketMode = mode;

	clientaddr_inGlobal = clientaddr_in;
	addrlenGlobal = addrlen;
	
	
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
		commandPost();
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

	int nc;

	switch(socketMode){
		case 0:
			if (send(sockfdGlobal, msg, lenGlobal, flagGlobal) != lenGlobal) erroutUtils(hostNameGlobal);
			break;
		case 1:

			nc = sendto (sockfdGlobal, msg, lenGlobal,
					0, (struct sockaddr *) &clientaddr_inGlobal, addrlenGlobal);
			if ( nc == -1) {
				perror("serverUDP: ");
				//exit(-1);
			}  
			break;
		case 2:
			printf("\n%s", msg);
			break;
	}
	
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

	colaCreaVacia(&c);

	char* token = strtok(bfCopy, character);

	while(token != NULL){
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

	char fileStart[] = "./articulos/";
	char *location;

	int index = 0;

	while(groupInput[index] != '\0'){
		if(groupInput[index] == '.'){
			groupInput[index] = '/';
		}
		index++;
	}

	location = malloc(strlen(fileStart) + strlen(groupInput) + 4);
	
	strcpy(location, fileStart);
	strcat(location, groupInput);
	
	DIR* d;
	struct dirent *dir;
	d = opendir(location);

	int first;
	int last;
	int count = 0;

	char *msg;
	if(d){
		while((dir = readdir(d)) != NULL){
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
		sendMsg("501 Error de sintaxis en GROUP newsgroup / 411 Grupo no encontrado");
	}


	selectedGroupPath = malloc(strlen(location) + 2);
	strcpy(selectedGroupPath, location);

	if(groupInput != NULL) free(groupInput);
	if(location != NULL) free(location);
}


void commandArticle(char *str){

	char *articleNumber;
	articleNumber = malloc(strlen(str) + 1);
	strcpy(articleNumber, str);

	char buff[lenGlobal];

	if(selectedGroupPath == NULL){
		sendMsg("412 No se ha seleccionado ningun grupo (GROUP)");
		return;
	} 

	DIR *d;
	struct dirent *dir;

	int articleFound = 0;

	d = opendir(selectedGroupPath);

	char *path;

	if(d){
		while((dir = readdir(d)) != NULL){
			if(strcmp(".", dir->d_name) && strcmp("..", dir->d_name)){

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
						articleFound = 1;
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

	if(!articleFound){
		sendMsg("423 Articulo no encontrado");
	}

}


void commandHead(char *str){

	char *articleNumber;
	articleNumber = malloc(strlen(str) + 1);
	strcpy(articleNumber, str);

	char buff[lenGlobal];

	if(selectedGroupPath == NULL){
		sendMsg("412 No se ha seleccionado ningun grupo (GROUP)");
		return;
	} 

	DIR *d;
	struct dirent *dir;

	int articleFound = 0;

	d = opendir(selectedGroupPath);

	char *path;

	if(d){
		while((dir = readdir(d)) != NULL){
			if(strcmp(".", dir->d_name) && strcmp("..", dir->d_name)){

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
					strcpy(initialMsg, "221 ");
					strcat(initialMsg, articleNumber);
					strcat(initialMsg, " cabeza recuperada");
					sendMsg(initialMsg);

					FILE *f;

					

					if((f = fopen(path, "r")) == NULL){
						perror("Error abriendo archivo");
					} else{
						articleFound = 1;
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

	if(!articleFound){
		sendMsg("423 Articulo no encontrado");
	}
}

void commandBody(char *str){

	char *articleNumber;
	articleNumber = malloc(strlen(str) + 1);
	strcpy(articleNumber, str);

	char buff[lenGlobal];

	if(selectedGroupPath == NULL){
		sendMsg("412 No se ha seleccionado ningun grupo (GROUP)");
		return;
	} 

	DIR *d;
	struct dirent *dir;

	int articleFound = 0;

	d = opendir(selectedGroupPath);

	char *path;

	if(d){
		while((dir = readdir(d)) != NULL){
			if(strcmp(".", dir->d_name) && strcmp("..", dir->d_name)){

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
					strcpy(initialMsg, "222 ");
					strcat(initialMsg, articleNumber);
					strcat(initialMsg, " cuerpo recuperado");
					sendMsg(initialMsg);

					FILE *f;					

					if((f = fopen(path, "r")) == NULL){
						perror("Error abriendo archivo");
					} else{
						fseek(f, 0, SEEK_SET);
						articleFound = 1;
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
	if(!articleFound){
		sendMsg("423 Articulo no encontrado");
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
	
	strcpy(location, fileStart);
	strcat(location, tema);
	strcat(location, "/");
	strcat(location, subtema);


	
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
						sendMsg(buff);
					}
					if(subject != NULL) free(subject); 
					if(id != NULL) free(id);
				}
				if(path != NULL)	free(path);
				
			}
		}
		closedir(d);

	} else{
		sendMsg("501 Error de sintaxis en GROUP newsgroup");
	}
}


void commandPost(){

	char buf[lenGlobal];
	char bufCopy[lenGlobal];
	char errorCause[200];
	char fileStart[] = "./articulos/";
	char *groupName;
	
	int i,j;
	int flag = 1;
	int lineCount = 0;
	int errorFound = 0;

	char *aux;

	int index;

	DIR *d;
	FILE *f;
	struct dirent *dir;
	char *location;

	int articleNumber = 1;


	sendMsg("240 Subiendo un articulo, finalice con una linea que solo contenga un punto");


	do{	

		switch(socketMode){
			case 0:
				i = recv(sockfdGlobal, buf, lenGlobal, flagGlobal);
				if (i == -1) {
					perror("ERROR RECV");
					exit(1);
				} else if(i == 0){//EOF
					flag = 0;
					break;
				}
				break;
			case 1:
				i = recvfrom(sockfdGlobal, buf, lenGlobal, 0,
					(struct sockaddr *)&clientaddr_inGlobal, &addrlenGlobal);
				if ( i == -1) {
					perror("Error recvfrom: ");
					printf("recvfrom error\n");
					exit (1);
					}
				break;
		}
		
		

		buf[i] = '\0';
		char *pos;
		if ((pos=strchr(buf, '\r')) != NULL)
		*pos = '\0';

		strcpy(bufCopy, buf);

		if(!errorFound){
			if(lineCount == 0){
				//Newsgroups, acceder a ruta y crear nuevo archivo
				Cola cM = splitLine(buf, " ");
				if(!colaVacia(&cM)){
					aux = colaSuprime(&cM);
					if(strcmp(aux, "Newsgroups:")){
						errorFound = 1;
						strcpy(errorCause, "No se ha detectado Newsgroups: en la primera linea");
					} else{
						if(!colaVacia(&cM)){
							aux = colaSuprime(&cM);

							groupName = malloc(strlen(aux) + 4);
							strcpy(groupName, aux);

							index = 0;
							while(aux[index] != '\0'){
								if(aux[index] == '.'){
									aux[index] = '/';
								}
								index++;
							}

							location = malloc(strlen(fileStart) + strlen(aux) + 10);
							strcpy(location, fileStart);
							strcat(location, aux);

							d = opendir(location);

							if(d){
								while((dir = readdir(d)) != NULL){
									if(strcmp(".", dir->d_name) && strcmp("..", dir->d_name)){
										int num = atoi(dir->d_name);
										if(num > articleNumber){
											articleNumber = num;
										}
									}
								}								

								closedir(d);

								articleNumber++;
								char *articleNumberStr = malloc(10);
								snprintf(articleNumberStr, 10, "/%d", articleNumber);

								strcat(location, articleNumberStr);

								if((f = fopen(location, "w+")) == NULL){
									errorFound = 1;
									strcpy(errorCause, "Error fopen");
									perror("fopen: ");
								} else{
									fputs(bufCopy, f);
									fputc('\n', f);
								}
							}

						} else{
							errorFound = 1;
							strcpy(errorCause, "No se ha detectado Newsgroups: (group) en la primera linea");
						}
					}
				} else{
					errorFound = 1;
					strcpy(errorCause, "No se ha detectado Newsgroups: (group) en la primera linea");
				}
			} else if(lineCount == 1){
				aux = strtok(buf, " ");

				if(aux == NULL || strcmp(aux, "Subject:")){
					errorFound = 1;
					strcpy(errorCause, "No se ha detectado Subject: en la segunda linea");
				} else{
					fputs(bufCopy, f);
					fputc('\n', f);

					//Colocamos Date despues de subject.
					time_t t = time(NULL);
					struct tm tms = *localtime(&t);
					snprintf(buf, lenGlobal, "Date: %d%02d%02d %02d%02d%02d", tms.tm_year - 100, tms.tm_mon +1, tms.tm_mday, tms.tm_hour, tms.tm_min, tms.tm_sec);
					fputs(buf, f);
					fputc('\n', f);

					//Colocamos el Message-ID

					snprintf(buf, lenGlobal, "Message-ID: <%d@%s>", articleNumber, hostNameGlobal);
					fputs(buf, f);
					fputc('\n', f);

				}
			} else{
				fputs(bufCopy, f);
				fputc('\n', f);
			}

			
		}
		lineCount++;
			
	} while(strcmp(bufCopy, ".") && flag);


	if(!errorFound){
		//Incrementamos el numero de articulos
		FILE *fAux, *fGrupos;
		
		if((fAux = fopen("n_articulos", "r")) == NULL){
			errorFound = 1;
			strcpy(errorCause, "Error fopen n_articulos");
			perror("fopen: ");
		} else{
			int num;
			fscanf(fAux, "%d", &num);
			num++;
			fclose(fAux);

			fAux = fopen("n_articulos", "w");
			fprintf(fAux, "%d", num);

			fclose(fAux);


			//Actualizamos el fichero grupos.

			struct grupo gStruc;

			int result;

			if((fGrupos = fopen("grupos", "r")) == NULL || (fAux = fopen("gruposAux", "w")) == NULL){
				perror("Error abriendo archivo");
			} else{
				while((result = fscanf(fGrupos, "%s %d %d %d %d %[^\n]", gStruc.loc, &gStruc.last, &gStruc.first, &gStruc.date, &gStruc.time, gStruc.descr)) != EOF){
					//result = 

					if(!strcmp(gStruc.loc, groupName)){
						fprintf(fAux, "%s %010d %010d %06d %06d %s\n", gStruc.loc, articleNumber, gStruc.first, gStruc.date, gStruc.time, gStruc.descr);			
					} else{
						fprintf(fAux, "%s %010d %010d %06d %06d %s\n", gStruc.loc, gStruc.last, gStruc.first, gStruc.date, gStruc.time, gStruc.descr);		
					}
					
				}
				fclose(fAux);
				fclose(fGrupos);

				if((fGrupos = fopen("grupos", "w")) == NULL || (fAux = fopen("gruposAux", "r")) == NULL){
					perror("Error abriendo archivo");
				} else{
					while((result = fscanf(fAux, "%s %d %d %d %d %[^\n]", gStruc.loc, &gStruc.last, &gStruc.first, &gStruc.date, &gStruc.time, gStruc.descr)) != EOF){

					fprintf(fGrupos, "%s %010d %010d %06d %06d %s\n", gStruc.loc, gStruc.last, gStruc.first, gStruc.date, gStruc.time, gStruc.descr);

					}
				}

				fclose(fAux);
				fclose(fGrupos);


			}
		}

		sendMsg("240 Articulo recibido correctamente.");
		//sendMsg(location);
	} else{
		sendMsg(errorCause);
	}

	if(f != NULL){
		fclose(f);
		if(errorFound){
			remove(location);
		}
	}

}




void writeLog(char *msg, int sem, char *prot, in_port_t port){

	char localProt[5];

	struct sembuf waitSem;
    waitSem.sem_num = 0;
    waitSem.sem_op = -1;
    waitSem.sem_flg = 0;
    
	struct sembuf signalSem;
    signalSem.sem_num = 0;
    signalSem.sem_op = 1;
    signalSem.sem_flg = 0;

	FILE *log;

	long timeVar;

	if(prot == NULL){
		switch(socketMode){
			case 0:
				strcpy(localProt, "TCP");
				break;
			case 1:
				strcpy(localProt, "UDP");
				break;	
		}
	} else{
		strcpy(localProt, prot);
	}

	if(port == -1){
		port = clientaddr_inGlobal.sin_port;
	}


	time(&timeVar);

	if(semop(sem, &waitSem, 1) == -1)
	{perror ("ERROR wait");
			exit(EXIT_FAILURE);
	} 

	if((log = fopen("serverLog", "a")) == NULL){
		perror("fopen log");
	} else{
		fprintf(log, "\n%s\t%s port %u: %s",(char *) ctime(&timeVar), localProt, ntohs(port), msg);
		fclose(log);
	}

	if(semop(sem, &signalSem, 1) == -1)
	{perror ("ERROR signal");
			exit(EXIT_FAILURE);
	}
}