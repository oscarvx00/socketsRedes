#include <stdio.h>
#include <stdlib.h>
#include "serverUtils.h"


int main(){

	char c[512] = "LIST";
	

	commandIn(c);
	
	printf("%s", c);
	
	return 0;
	
}

