#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "cola.h"



int colaCreaVacia(Cola *c)
{
    if(colaVacia(c))	return -1;
    
    c->frente = c->fondo = NULL;
    return 0;
    
}

int colaVacia(Cola *c)
{
    if(c == NULL)		return -1;
    if(c->frente == NULL)	return 1;
    
    return 0;
    
}

int colaInserta(Cola *c,tipoElemento elemento)
{
	if(c == NULL)		return -2;
	
	tipoCelda *nuevo;
	if((nuevo = malloc(sizeof(tipoCelda))) == NULL)		return -1;
	
	nuevo->elemento = elemento;
	nuevo->sig = NULL;
	
	
	if(c->fondo != NULL)	c->fondo->sig = nuevo;
	c->fondo = nuevo;
	if(c->frente == NULL)	c->frente = c->fondo;
	
	return 0;
	
    
}

tipoElemento colaSuprime(Cola *c)
{
    if(c == NULL)			return -2;
    if(c->frente == NULL)	return -3;
    
    tipoCelda *aBorrar;
    
    aBorrar = c->frente;
	c->frente = c->frente->sig;
	if(c->fondo == aBorrar)	c->fondo = c->frente;
	
	tipoElemento el = aBorrar->elemento;
	
	free(aBorrar);
	
	return el;
    
}
