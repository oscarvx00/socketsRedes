#ifndef SERVERUTILS_H
#define SERVERUTILS_H

#include "cola.h"

struct grupo{
    char loc[512];
    int first;
    int last;
    int date;
    int time;
    char descr[512];
};


int commandIn(int sockfd, char *bf, size_t len, int flags, char* hostname);
void sendMsg(char* msg);

Cola splitLine(char *bf, char *character);

void commandList();
void commandGroup(char* str);
void commandArticle(char *str);
void commandHead(char *str);
void commandBody(char *str);
void commandNewGroups(char *date, char *time);
void commandNewNews(char *location, char *date, char *time);
void commandPost();

void erroutUtils(char *);		/* declare error out routine */


#endif
