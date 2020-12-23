#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "serverUtils.h"
#include "cola.h"


int main(){


			struct sockaddr dummy2;
			int dummy3 = 0;

	char c[512] = "GROUP c.c";
	char *pos;

	int flag = 1;
	while(flag){
		fgets(c, 512, stdin);
		if ((pos=strchr(c, '\n')) != NULL)
		*pos = '\0';
		
		flag = commandIn(0,c,512,0,"NONAME", 2, dummy2, dummy3);
	}


	
	return 0;
	
}

