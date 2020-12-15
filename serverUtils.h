#ifndef SERVERUTILS_H
#define SERVERUTILS_H

#include "cola.h"

int commandIn(int sockfd, char *bf, size_t len, int flags, char* hostname);
Cola splitCommand(char *bf);
void sendMsg(char* msg);

void commandList();
void commandNewgroups(Cola c);
void commandNewnews(Cola c);
void commandGroup(char* str);
Cola splitGroup(char *bf);

void erroutUtils(char *);		/* declare error out routine */


#endif
