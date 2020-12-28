
all: servidor cliente

servidor: servidor.c serverUtils.o serverUtils.h
	gcc -g servidor.c serverUtils.o cola.o -o servidor
	
serverUtils.o: serverUtils.h serverUtils.c cola.o
	gcc -g -c serverUtils.c
	
cola.o: cola.h cola.c
	gcc -g -c cola.c


cliente: cliente.c
	gcc -g -o cliente cliente.c
