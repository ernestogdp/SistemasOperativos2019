#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <err.h>
#include <fcntl.h>

typedef struct process
{
	pid_t p;
  	int unique;
  	char fich[100];
} process;
typedef struct process process;

int waitexitcmp(){
	pid_t wpid;
	int sts;
	int result=2;
	int different=0;

	while((wpid= wait(&sts)) != -1){
		if (WIFEXITED(sts)) {
			switch(WEXITSTATUS(sts)){
			case 0:
				//same file
				result=0;
				break;
			case 1:
				//diferent
				different=1;
				break;
			case -1:
				printf("error son!\n");
			}
		}
	}
	if(different && result==2){
		result=1;
	}
	return result;
}

void execcmp(char *fichk, char *fich){
	execl("/usr/bin/cmp", "cmp", "-s", fichk, fich, NULL);
}

void cmpforeachfile(char *fichkey, char *fichs[], int fichkeyposition, int numfichtocmp){
	pid_t pid;

	for(int i=0; i<numfichtocmp; i++){
		if(i != fichkeyposition){
			pid=fork();
			switch(pid){
			case -1:
				err(EXIT_FAILURE, "fork failed!");
			case 0:
				execcmp(fichkey, fichs[i]);
				err(EXIT_FAILURE, "error on exit grandchild");
			}
		}
	}
	exit(waitexitcmp());
}

int managecmpresults(process results[], int numresults){
	int existduplicates=0;

	for(int i=0; i<numresults; i++){
		if(results[i].unique==2){
			errx(EXIT_FAILURE, "usage: uniqfiles[files ...]");
		}
	}
	for(int j=0; j<numresults; j++){
		if(results[j].unique==1){
			printf("%s\n", results[j].fich);
		}else{
			existduplicates=1;
		}
	}
	return existduplicates;
}

int processcmp(int numarg, char *arg[]){
	pid_t pid;
	pid_t wpid;
	int sts;
	process sontraits[numarg];

	for(int i=0; i<numarg; i++){
		pid=fork();
		switch(pid){
		case -1:
			err(EXIT_FAILURE, "fork failed!");
		case 0:
			cmpforeachfile(arg[i], arg, i, numarg);
			err(EXIT_FAILURE, "error on exit son");
		default:
			sontraits[i].p=pid;
			strcpy(sontraits[i].fich, arg[i]);
		}
	}

	while((wpid= wait(&sts)) != -1){
		if (WIFEXITED(sts)) {
			if(WEXITSTATUS(sts)==-1){
				printf("error on exit son");
			}else{
				for(int i=0; i<numarg; i++){
					if(wpid==sontraits[i].p){
						sontraits[i].unique=WEXITSTATUS(sts);
					}
				}
			}
		}
	}
	return managecmpresults(sontraits, numarg);
}


int
main(int argc, char *argv[])
{
	int exitresult=0;

	if((argc-1) == 0){
		err(EXIT_FAILURE, "usage: uniqfiles[files ...]");
	}
	exitresult=processcmp(--argc, ++argv);

	exit(exitresult);
}
