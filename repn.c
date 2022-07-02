#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>

enum{
		LETTERS=5
};

int checkifflagr(int numarg, char *arg[]){
	if(strcmp(arg[1], "-r")==0){
		for (int i=1; i<numarg; i++){
			arg[i]=arg[i+1];
		}
		return 1;
	}else{
		return 0;
	}
}

void cutword(int nword, char *word[]){

	for (int i=2; i<nword; i++){
		if (strlen(word[i])>LETTERS){
			word[i][LETTERS]='\0';
		}
	}

}

void printrepeatword(int numword, char *words[]){

	for(int i=2; i<numword; i++){
		for(int j=0; j<atoi(words[1]); j++){
				printf("%s", words[i]);
		}
		printf("\n");
	}

}

void printreverse(int narg, char *arg[]){

	int longword=LETTERS;

	for (int i=2; i<narg; i++){
		if(strlen(arg[i])!=LETTERS){
			longword=strlen(arg[i]);
		}
		for(int z=0; z<atoi(arg[1]); z++){
			for (int j=longword; j>=0;j--){
				printf ("%c", arg[i][j]);
			}
		}
		printf("\n");
	}
}

int main(int argc, char *argv[]){

	int flagr=0;

	if (argc < 2){
		fprintf(stderr, "usage: repn [-r] n [string ...]\n");
		exit(EXIT_FAILURE);
	}
	flagr=checkifflagr(argc, argv);
	if (argc == 2)
		exit(EXIT_SUCCESS);
	if (flagr)
		argc--;
	cutword(argc, argv);
	if(flagr){
		printreverse(argc, argv);
	}else{
		printrepeatword(argc, argv);
	}

	exit(EXIT_SUCCESS);
}
