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
  	char word[100];
} process;
typedef struct process process;

void sonwork(char *fsearch, char *keyword){
	execl("/bin/fgrep", "fgrep", "-q", "-s", keyword, fsearch, NULL);
}

void processsonexit(process procinfo[], int numprocesses){
	pid_t wpid;
	int sts;
	char *keyword;
	while((wpid= wait(&sts)) != -1){
		for(int i=0; i<numprocesses; i++){
			if(wpid==procinfo[i].p){
				keyword=procinfo[i].word;
			}
		}
		if (WIFEXITED(sts)) {
			switch(WEXITSTATUS(sts)){
			case 0:
				printf("%s: si\n", keyword);
				break;
			case 1:
				printf("%s: no\n", keyword);
				break;
			case 2:
				printf("%s: error\n", keyword);
				break;
			default:
				errx(EXIT_FAILURE, "error");
			}
		}
	}
}

void processfgrep(int numarg, char *arg[]){
	pid_t pid;
	int processes=numarg/2;
	process sontraits[processes];

	for(int i=0; i<processes; i++){
		pid=fork();
		switch(pid){
		case -1:
			err(EXIT_FAILURE, "fork failed!");
		case 0:
			sonwork(arg[i*2], arg[i*2+1]);
			err(EXIT_FAILURE, "error on exit son");
		default:
			sontraits[i].p=pid;
			strncpy(sontraits[i].word, arg[i*2+1], 100);
		}
	}
	processsonexit(sontraits, processes);

}


int
main(int argc, char *argv[])
{
	if((argc-1) % 2 != 0){
		err(EXIT_FAILURE, "usage: hasword[[file word]...]");
	}
	processfgrep(--argc, ++argv);

	exit(EXIT_SUCCESS);
}
