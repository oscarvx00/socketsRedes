
test: test.c serverUtils.o serverUtils.h cola.h cola.o
	gcc -g test.c serverUtils.o cola.o -o test

servidor: servidor.c serverUtils.o serverUtils.h
	gcc -g servidor.c serverUtils.o cola.o -o server
	
serverUtils.o: serverUtils.h serverUtils.c cola.o
	gcc -g -c serverUtils.c
	
cola.o: cola.h cola.c
	gcc -g -c cola.c
