#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "linkedList.h"

typedef struct no no_t;

struct no
{
	elem info;
	struct no *prox;
};

struct lista
{	
	struct no *inicio, *fim;
	int tamanho;
};

lista_t *criar()
{
	lista_t *l = (lista_t *) malloc(sizeof(lista_t));
	assert(l != NULL);

	l->inicio = NULL;
	l->fim = NULL;
	l->tamanho = 0;

	return l;
}

void insere(lista_t *l, elem x)
{
	assert(l != NULL);

	// Nó p toma o lugar do última fim da lista e p->prox aponta para o
	//	inicio da lista circular
	no_t *p = (no_t *) malloc(sizeof(no_t));
	p->info = x;

	if(l->inicio == NULL)
	{
		l->inicio = p;
		p->prox = l->inicio;
		l->fim = p;
	}
	else
	{
		l->fim->prox = p;
		//p->prox = l->inicio;
		l->fim = p;
	}

	l->tamanho++;
}

void remover(lista_t *l, elem x)
{
	no_t *p = l->inicio;
	no_t *ant = l->fim;

	int pos = 0;
	// Pula os primeiros x nós
	while(pos++ < x)
	{
		ant = p;
		p = p->prox;
	}

	if(p == l->inicio)
	{
		l->inicio = l->inicio->prox;
		free(p);
		l->fim->prox = l->inicio;
	}
	else
	{
		ant->prox = p->prox;
		free(p);
		p = ant->prox;
		
		// Muda o primeiro nó da fila para o p
		//l->inicio = p;
		//l->fim = ant;
	}

	l->tamanho--;
}
/*
void imprime(lista_t *l)
{
	no_t *p = l->inicio;
	int i=0;
	while(i++ < tamanho(l))
		printf("%d", p->info);
}*/

int tamanho(lista_t *l)
{
	assert(l != NULL);

	return l->tamanho;
}

void libera(lista_t *l)
{
	no_t *p;
	if(l != NULL)
	{
		p = l->inicio;
		while(l->tamanho > 0)
		{
			l->inicio = p->prox;
			free(p);
			p = l->inicio;
			l->tamanho--;
		}
		free(l);
	}
}