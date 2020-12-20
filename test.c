#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "serverUtils.h"
#include "cola.h"


int main(){

	char c[512] = "GROUP c.c";
	char *pos;

	int flag = 1;
	while(flag){
		fgets(c, 512, stdin);
		if ((pos=strchr(c, '\n')) != NULL)
		*pos = '\0';
		
		flag = commandIn(0,"NEWNEWS local.redes 20200000 000000",512,0,"NONAME");
	}


	
	return 0;
	
}

