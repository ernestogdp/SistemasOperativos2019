#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct nodo_s
{
  	char dato[31];
  	struct nodo_s *siguiente;
} nodo_t;

typedef nodo_t *ptrNodo;
typedef nodo_t *Stack;

int push(Stack *s, char *str)
{
  	ptrNodo nuevo;

  	nuevo = (ptrNodo)malloc(sizeof(nodo_t));
	if(!nuevo) return -1;
	strncpy(nuevo->dato, str, 31);
	nuevo->siguiente = *s;
	*s=nuevo;

	return 1;
}

char* pop(Stack *s)
{

  	ptrNodo nodo;
  	char *str;

  	nodo = *s;
  	if(!nodo) return NULL;

  	*s = nodo->siguiente;
  	str = strdup(nodo->dato);

  	free(nodo);

  	return str;
}

int drop(Stack *s, int n){

	ptrNodo nodo;

	for(int i=0; i<n; i++){
		nodo = *s;
		if(!nodo) return -1;
		*s =nodo->siguiente;
		free(nodo);
	}
	return 1;
}

int isempty(Stack *s){
	if(!s){
		return 1;
	}else{
		return 0;
	}
}

int main (int argc, char *argv[])
{
  	Stack s = NULL;
  	char a;
  	char b;

  	for(a='a';a<='z';a++){
    	for(b='a';b<='z';b++){
      		char cadena[] = {a, b, '\0'};
      		push(&s, cadena);
    	}
  	}

  	while (s != NULL){
    	printf("%s\n", pop(&s));
		if (strcmp(&s->dato[0], &s->dato[1])==0){
			drop(&s, 4);
		}
  	}
}
