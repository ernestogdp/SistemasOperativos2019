#include <stdlib.h>
#include <stdio.h>
#include <string.h>

enum{
	MAXCHAR=40;
	MAXSTACK=5;
};

struct Stack{
	char *list[MAXCHAR];
	char *next;
}

void initstack(Stack *s){
	for(i=0;i<MAXCHAR; i++){
		s.list[i]=NULL;
	}
	s.next=NULL;
}

void push(char *str, Stack *s){



}

char* pop(Stack *s){



}*/





int main(int argc, char* argv[]){

	struct Stack mystack[MAXSTACK];
	char str[]="ab";
	char str1[]="bc";
	char str2]="ad";
	char str3[]="cb";

	for(int i=0;i<MAXSTACK;i++){
		initstack(mystack[i]);
	}
	//cleanarray(mystack.list);
	push(str, mystack);
	push(str1, mystack);
	push(str2, mystack);
	push(str3, mystack);



}
