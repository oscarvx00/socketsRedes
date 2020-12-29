/*
 *			C L I E N T C P
 *
 *	This is an example program that demonstrates the use of
 *	stream sockets as an IPC mechanism.  This contains the client,
 *	and is intended to operate in conjunction with the server
 *	program.  Together, these two programs
 *	demonstrate many of the features of sockets, as well as good
 *	conventions for using these features.
 *
 *
 */
 
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <time.h>

#include <signal.h>
#include <sys/errno.h>


#define PUERTO 8438
#define TAM_BUFFER 512


#define ADDRNOTFOUND	0xffffffff	/* value returned for unknown host */
#define RETRIES	5		/* number of times to retry before givin up */
#define TIMEOUT 6
#define MAXHOST 512

#define MODE_MANUAL 0
#define MODE_FILE 1

void clientTCP(char *hostN);
void clientUDP();

void handler()
{
 printf("Alarma recibida \n");
}


void functionPostTCP(int s);
void functionPostUDP(int s, struct sockaddr_in serverAddr, struct sockaddr_in clientAddr);


FILE *f;
FILE *fLog;
char *fLogName;
int mode;


/*
 *			M A I N
 *
 *	This routine is the client which request service from the remote.
 *	It creates a connection, sends a number of
 *	requests, shuts down the connection in one direction to signal the
 *	server about the end of data, and then receives all of the responses.
 *	Status will be written to stdout.
 *
 *	The name of the system to which the requests will be sent is given
 *	as a parameter to the command.
 */
int main(argc, argv)
int argc;
char *argv[];
{
    
	if(argc == 3){
		mode = MODE_MANUAL;
	} else if(argc == 4){
		mode = MODE_FILE;
		if((f = fopen(argv[3], "r")) == NULL){
		fprintf(stderr, "Error abriendo fichero de ordenes");
		exit(1);
	}
	} else{
		fprintf(stderr, "Usage:  %s <remote host> <TCP/UDP> <ficheroOrdenes.txt> (Si no se proporciona fichero de ordenes será modo manual)\n", argv[0]);
		exit(1);
	}

	

    if(!strcmp("TCP", argv[2]))
        clientTCP(argv[1]);
    else if (!strcmp("UDP", argv[2]))
        clientUDP(argv[1]);
    else{
        fprintf(stderr, "Usage:  %s <remote host> <TCP/UDP> >ficheroOrdenes.txt>\n", argv[0]);
		exit(1);
    }

	if(mode == MODE_FILE){
		fclose(fLog);
		fclose(f);
	}

	return 0;
}




void clientTCP(char *hostN){


    int s;				/* connected socket descriptor */
   	struct addrinfo hints, *res;
    long timevar;			/* contains time returned by time() */
    struct sockaddr_in myaddr_in;	/* for local socket address */
    struct sockaddr_in servaddr_in;	/* for server socket address */
	int addrlen, i, j, errcode;
    /* This example uses TAM_BUFFER byte messages. */
	char buf[TAM_BUFFER];

    char hostName[80];




    strcpy(hostName, hostN);


    	/* Create the socket. */
	s = socket (AF_INET, SOCK_STREAM, 0);
	if (s == -1) {
		perror("Error creando socket TCP");
		fprintf(stderr, "unable to create socket\n");
		exit(1);
	}
	
	/* clear out address structures */
	memset ((char *)&myaddr_in, 0, sizeof(struct sockaddr_in));
	memset ((char *)&servaddr_in, 0, sizeof(struct sockaddr_in));

	/* Set up the peer address to which we will connect. */
	servaddr_in.sin_family = AF_INET;
	
	/* Get the host information for the hostname that the
	 * user passed in. */
      memset (&hints, 0, sizeof (hints));
      hints.ai_family = AF_INET;
 	 /* esta funci�n es la recomendada para la compatibilidad con IPv6 gethostbyname queda obsoleta*/
    errcode = getaddrinfo (hostName, NULL, &hints, &res); 
    if (errcode != 0){
			/* Name was not found.  Return a
			 * special value signifying the error. */
		fprintf(stderr, "No es posible resolver la IP de %s\n", hostName);
		exit(1);
        }
    else {
		/* Copy address of host */
		servaddr_in.sin_addr = ((struct sockaddr_in *) res->ai_addr)->sin_addr;
	    }
    freeaddrinfo(res);

    /* puerto del servidor en orden de red*/
	servaddr_in.sin_port = htons(PUERTO);

		/* Try to connect to the remote server at the address
		 * which was just built into peeraddr.
		 */

	
	
	if (connect(s, (const struct sockaddr *)&servaddr_in, sizeof(struct sockaddr_in)) == -1) {
		perror("Error connect TCP: ");
		fprintf(stderr, "unable to connect to remote\n");
		exit(1);
	}
		/* Since the connect call assigns a free address
		 * to the local end of this connection, let's use
		 * getsockname to see what it assigned.  Note that
		 * addrlen needs to be passed in as a pointer,
		 * because getsockname returns the actual length
		 * of the address.
		 */


	addrlen = sizeof(struct sockaddr_in);
	if (getsockname(s, (struct sockaddr *)&myaddr_in, &addrlen) == -1) {
		perror("Error getsockname TCP: ");
		fprintf(stderr, "unable to read socket address\n");
		exit(1);
	 }

	/* Print out a startup message for the user. */
	time(&timevar);
	/* The port number must be converted first to host byte
	 * order before printing.  On most hosts, this is not
	 * necessary, but the ntohs() call is included here so
	 * that this program could easily be ported to a host
	 * that does require it.
	 */

	

	int flag = 1;

	if(mode == MODE_FILE){
		fLogName = malloc(20);
		sprintf(fLogName, "%u.txt", ntohs(myaddr_in.sin_port));
		printf("LOG NAME: %s", fLogName);

		if((fLog = fopen(fLogName, "w")) == NULL){
			printf("\nError abriendo archivo de log");
			exit(-1);
		}
		free(fLogName);	
	}	

	if(mode == MODE_MANUAL){
		printf("Connected to %s on port %u at %s",
			hostName, ntohs(myaddr_in.sin_port), (char *) ctime(&timevar));
	} else{
		fprintf(fLog, "Connected to %s on port %u at %s",
			hostName, ntohs(myaddr_in.sin_port), (char *) ctime(&timevar));
	}
	

	while(flag){

	if(mode == MODE_MANUAL){
	
		printf("\n\nIntroduce comando: ");
		char *pos;
		fgets(buf, 512, stdin);
		if ((pos=strchr(buf, '\n')) != NULL)
		*pos = '\0';
	} else{
		if(fgets(buf, TAM_BUFFER, f) == NULL){
			if(!feof(f)){
				perror("Error leyendo archivo de ordenes");
				return;
			}
		}
	}

			if (send(s, buf, TAM_BUFFER, 0) != TAM_BUFFER) {
				fprintf(stderr, "Connection aborted on error ");
				fprintf(stderr, "on send number %d\n", i);
				exit(1);
			}

			if(!strcmp(buf, "POST\r\n") || !!strcmp(buf, "POST")){
				//POSTING
				functionPostTCP(s);
			} else if(!strcmp("QUIT\r\n", buf) || !strcmp("QUIT", buf)){
				flag = 0;
			}

		do{	
			i = recv(s, buf, TAM_BUFFER, 0);
			if (i == -1) {
				perror("Error recv TCP: ");
				fprintf(stderr, "error reading result\n");
				exit(1);
			} else if(i == 0){//EOF
				flag = 0;
				break;
			}
				/* The reason this while loop exists is that there
				* is a remote possibility of the above recv returning
				* less than TAM_BUFFER bytes.  This is because a recv returns
				* as soon as there is some data, and will not wait for
				* all of the requested data to arrive.  Since TAM_BUFFER bytes
				* is relatively small compared to the allowed TCP
				* packet sizes, a partial receive is unlikely.  If
				* this example had used 2048 bytes requests instead,
				* a partial receive would be far more likely.
				* This loop will keep receiving until all TAM_BUFFER bytes
				* have been received, thus guaranteeing that the
				* next recv at the top of the loop will start at
				* the begining of the next reply.
				*/
			while (i < TAM_BUFFER) {
				j = recv(s, &buf[i], TAM_BUFFER-i, 0);
				if (j == -1) {
						perror("Error recv TCP: ");
						fprintf(stderr, "error reading result\n");
						exit(1);
				}
				i += j;
			}
			if(strcmp(buf, "\r\n")){
				if(mode == MODE_MANUAL){
					printf("\nC: %s", buf);
				} else{
					fprintf(fLog, "\n%s", buf);
				}
			}	
				
		} while(strcmp(buf, "\r\n"));
				/* Print out message indicating the identity of this reply. */
			//printf("Received result number %d\n", *buf);
			
		//}
	}

	close(s);

    /* Print message indicating completion of task. */
	time(&timevar);
	if(mode == MODE_MANUAL){
		printf("All done at %s", (char *)ctime(&timevar));
	} else{
		fprintf(fLog, "All done at %s", (char *)ctime(&timevar));
	}

}


void functionPostTCP(int s){

	char buf[TAM_BUFFER];
	char *pos;

	int i, j;
	//Recibimos mensaje de confirmacion

	i = recv(s, buf, TAM_BUFFER, 0);
			if (i == -1) {
				//perror(argv[0]);
				//fprintf(stderr, "%s: error reading result\n", argv[0]);
				exit(1);
			} else if(i == 0){//EOF
				//flag = 0;
				//break;
			}
				/* The reason this while loop exists is that there
				* is a remote possibility of the above recv returning
				* less than TAM_BUFFER bytes.  This is because a recv returns
				* as soon as there is some data, and will not wait for
				* all of the requested data to arrive.  Since TAM_BUFFER bytes
				* is relatively small compared to the allowed TCP
				* packet sizes, a partial receive is unlikely.  If
				* this example had used 2048 bytes requests instead,
				* a partial receive would be far more likely.
				* This loop will keep receiving until all TAM_BUFFER bytes
				* have been received, thus guaranteeing that the
				* next recv at the top of the loop will start at
				* the begining of the next reply.
				*/
			while (i < TAM_BUFFER) {
				j = recv(s, &buf[i], TAM_BUFFER-i, 0);
				if (j == -1) {
						//perror(argv[0]);
						//fprintf(stderr, "%s: error reading result\n", argv[0]);
						exit(1);
				}
				i += j;
			}
			if(mode == MODE_MANUAL){
				printf("\nC: %s", buf);
			} else{
				fprintf(fLog, "\n%s", buf);
			}

	do{
		if(mode == MODE_MANUAL){
		
			printf("\n\nIntroduce comando: ");
			char *pos;
			fgets(buf, 512, stdin);
			if ((pos=strchr(buf, '\n')) != NULL)
			*pos = '\0';
		} else{
			if(fgets(buf, TAM_BUFFER, f) == NULL){
				if(!feof(f)){
				perror("Error leyendo archivo de ordenes");
				return;
			}
			}
		}


			if (send(s, buf, TAM_BUFFER, 0) != TAM_BUFFER) {
				//fprintf(stderr, "%s: Connection aborted on error ",	argv[0]);
				//fprintf(stderr, "on send number %d\n", i);
				exit(1);
			}
	} while(strcmp(buf, ".\r\n"));

}


void clientUDP(char *hostN){


	char buf[TAM_BUFFER];
	char *hostName;

	int i;
	int s;
	int errcode;

	struct sockaddr_in serverAddr, clientAddr;
	struct hostent *hp;
	struct in_addr reqaddr;
	socklen_t len;

	struct addrinfo hints, *res;

	hostName = malloc(strlen(hostN) + 4);
	strcpy(hostName, hostN);



	memset(&clientAddr, 0, sizeof(struct sockaddr_in));
	memset(&serverAddr, 0, sizeof(struct sockaddr_in));
	


	if((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
		perror("Error creando socket: ");
		exit(-1);
	}

	
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;

	if(getaddrinfo(hostName, NULL, &hints, &res) != 0){
		perror("Error getaddrinfo");
		exit(-1);
	} else{
		reqaddr = ((struct sockaddr_in *) res->ai_addr)->sin_addr;
	}

	


	bzero((char *) &clientAddr, sizeof(clientAddr));

	len = sizeof(clientAddr);
	if (getsockname(s, (struct sockaddr *)&clientAddr, &len) == -1) {
		perror("Error getsockname TCP: ");
		fprintf(stderr, "unable to read socket address\n");
		exit(1);
	 }

	if(bind(s, (struct sockaddr *) &clientAddr, sizeof(clientAddr)) < 0){
		perror("Error bind: ");
		close(s);
		exit(-1);
	}






	bzero((char *) &serverAddr, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr = ((struct sockaddr_in *) res->ai_addr)->sin_addr;
	serverAddr.sin_port = htons(PUERTO);

	freeaddrinfo(res);

	if(mode == MODE_FILE){
		fLogName = malloc(20);
		sprintf(fLogName, "%u.txt", ntohs(clientAddr.sin_port));
		printf("LOG NAME: %s", fLogName);

		if((fLog = fopen(fLogName, "w")) == NULL){
			printf("\nError abriendo archivo de log");
			exit(-1);
		}
		free(fLogName);	
	}

	
	int flag = 1;
	
	while(flag){

		if(mode == MODE_MANUAL){
		
			printf("\n\nIntroduce comando: ");
			char *pos;
			fgets(buf, 512, stdin);
			if ((pos=strchr(buf, '\n')) != NULL)
			*pos = '\0';
		} else{
			if(fgets(buf, TAM_BUFFER, f) == NULL){
				if(!feof(f)){
				perror("Error leyendo archivo de ordenes");
				return;
			}
			}
		}



		if (sendto (s, buf, TAM_BUFFER, 0, (struct sockaddr *)&serverAddr,
				sizeof(serverAddr)) == -1) {
        		perror("Error sendto: ");
        		fprintf(stderr, "unable to send request\n");
        		exit(1);
        	}

		if(!strcmp(buf, "POST\r\n") || !strcmp(buf, "POST\r\n")){
				//POSTING
				functionPostUDP(s, serverAddr, clientAddr);
		} else if(!strcmp("QUIT\r\n", buf) || !strcmp("QUIT", buf)){
			flag = 0;
		}

		do{

			i = recvfrom(s, buf, TAM_BUFFER, 0, (struct sockaddr *) &clientAddr, &len);
			buf[i] = '\0';


			if(strcmp(buf, "\r\n")){
				if(mode == MODE_MANUAL){
					printf("\nC: %s", buf);
				} else{
					fprintf(fLog, "\n%s", buf);
				}
			}

		} while(strcmp(buf, "\r\n"));


	}

	close(s);

}


void functionPostUDP(int s, struct sockaddr_in serverAddr, struct sockaddr_in clientAddr){

	char buf[TAM_BUFFER];
	char *pos;

	struct sockaddr_in sAddr, cAddr;
	sAddr = serverAddr;
	cAddr = clientAddr;

	socklen_t len = sizeof(cAddr);
	int i, j;

	//Recibimos mensaje de confirmacion

	i = recvfrom(s, buf, TAM_BUFFER, 0, (struct sockaddr *) &cAddr, &len);
			if (i == -1) {
				perror("Error recvfrom function post");
				//fprintf(stderr, "%s: error reading result\n", argv[0]);
				exit(1);
			} else if(i == 0){//EOF
				//printf("CLIENTE SALE");
				//break;
			}
	if(mode == MODE_MANUAL){
		printf("\nC: %s", buf);
	} else{
		fprintf(fLog, "\n%s", buf);
	}

	do{
		if(mode == MODE_MANUAL){
		
			char *pos;
			fgets(buf, 512, stdin);
			if ((pos=strchr(buf, '\n')) != NULL)
			*pos = '\0';
		} else{
			if(fgets(buf, TAM_BUFFER, f) == NULL){
				if(!feof(f)){
				perror("Error leyendo archivo de ordenes");
				return;
			}
			}
		}


			if (sendto (s, buf, TAM_BUFFER, 0, (struct sockaddr *)&sAddr,
				sizeof(sAddr)) == -1) {
        		perror("Error sendto: ");
        		fprintf(stderr, "unable to send request\n");
        		exit(1);
        	}
	} while(strcmp(buf, ".\r\n"));

}
