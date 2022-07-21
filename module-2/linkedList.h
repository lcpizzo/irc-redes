#ifndef _LINKEDLIST_H_
#define _LINKEDLIST_H_

typedef void *elem;

typedef struct lista lista_t;

// Cria uma lista circular
lista_t *criar();
// Insere um elemento x na última posição da lista circular (a que 
//	aponta de volta pro inicio)
void insere(lista_t *L, elem x);
// Remove o elemento na posição (índice) a partir do inicio da 
//	lista circular 
void remover(lista_t *L, elem x);
// Checa o tamanho da estrutura
int tamanho(lista_t *L);
// Libera a memória alocada
void libera(lista_t *L);
// Imprime a lista circular inteira
//void imprime(lista_t *L);

#endif