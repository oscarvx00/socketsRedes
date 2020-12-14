#ifndef SERVERUTILS_H
#define SERVERUTILS_H

#include "cola.h"

void commandIn(int sockfd, char *bf, size_t len, int flags, char* hostname);
Cola splitCommand(char *bf);
void sendMsg(char* msg);

void commandList();

void erroutUtils(char *);		/* declare error out routine */


#endif
