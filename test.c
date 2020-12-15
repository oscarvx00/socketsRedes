#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "serverUtils.h"


int main(){

	char c[512] = "GROUP c.c";
	char *pos;
	fgets(c, 512, stdin);
	if ((pos=strchr(c, '\n')) != NULL)
    *pos = '\0';
	
	printf("%s", c);

	printf("COMP %d", strcmp(c, "GROUP c.c"));
	//gets(c);
	char copy[512];
	strcpy(copy, c);
	commandIn(0,copy,512,0,"NONAME");

	
	return 0;
	
}

