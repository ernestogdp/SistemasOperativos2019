#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <err.h>
#include <glob.h>
#include <errno.h>

enum{
	MAXPROCESSES = 3,
	MAXNUMARG = 10,
	SIZE = 1024,
	VERBOSE = 0
};

struct Pipelist{
	int fd[2];
};

struct Processes{
	char *command;
	char *arg[MAXNUMARG+1];
	int numarg;
};

typedef struct Processes Processes;
typedef struct Pipelist Pipelist;

int extractwords(char *buffer, char *divider, char **tokens)
{
	int i=0;
	char *t;

	t= strtok(buffer, divider);
	while(t){
		tokens[i]=t;
		t=strtok(NULL, divider);
		i++;
	}
	return i;
}

void closepipes(Pipelist pipe[], int numpipes){

	for(int z=0; z<numpipes; z++){
		for(int k=0; k<2; k++){
			if(close(pipe[z].fd[k]) == -1)
				err(EXIT_FAILURE, "Failed to close");
		}
	}
}

void setprocesses(Processes p[], char *lines[]){

	for(int i=0; i<MAXPROCESSES; i++){
		p[i].numarg=extractwords(lines[i], " ", p[i].arg);
		p[i].command=p[i].arg[0];
	}

	if(VERBOSE){
		for(int j=0; j<MAXPROCESSES; j++) {
			printf("comando: ([%s]) | argumentos [%d]: ", p[j].command, p[j].numarg);
			for(int k=0; k<p[j].numarg; k++){
				printf("[%s] ", p[j].arg[k]);
			}
			printf("\n");
		}
	}
}

int makeexecutable(char *ex, char *cmd){

	char *ejecpath[]= {"/bin", "/usr/bin"};
	int find=0;

	for(int i=0; i<2; i++){
		sprintf(ex, "%s/%s", ejecpath[i], cmd);
		if(access(ex, X_OK) == 0){
			find=1;
			break;
		}
	}

	if(!find)
		fprintf(stderr, "The command [[%s] could not be found\n", cmd);

	return find;

}

void changeentryandexit(Pipelist pipe[], int numprocess){

	switch(numprocess){
	case 0:
		dup2(pipe[numprocess].fd[1], 1);
		break;
	case MAXPROCESSES-1:
		dup2(pipe[numprocess-1].fd[0], 0);
		break;
	default:
		dup2(pipe[numprocess-1].fd[0], 0);
		dup2(pipe[numprocess].fd[1], 1);
		break;
	}

	closepipes(pipe, MAXPROCESSES-1);
}

void execcommand(Processes proc, Pipelist pipelist[], int processcount){

	char executable[SIZE];

	if(!makeexecutable(executable, proc.command))
		err(EXIT_FAILURE, "aborted process");

	changeentryandexit(pipelist, processcount);

	//Insertamos el último valor de argv -> (NULL)
	proc.arg[proc.numarg]=NULL;
	execv(executable, proc.arg);
	err(EXIT_FAILURE, "%s failed!", proc.command);

}

int waitsons(pid_t *pid){
	int sts;
	int exitflag=1;

	for(int i=0; i<MAXPROCESSES; i++){
		while((waitpid(pid[i],&sts,0)) != -1){
			if(WIFEXITED(sts)){
				if(i==MAXPROCESSES-1)
					exitflag=WEXITSTATUS(sts);
			}else{
				printf("Error on exit son with pid: %d\n", pid[i]);
			}
		}
	}

	return exitflag;
}

int makeprocesses(Processes p[]){

	Pipelist plist[MAXPROCESSES-1];
	pid_t pid[MAXPROCESSES];
	int exitlastson;


	for(int i=0; i<MAXPROCESSES-1; i++){
		if(pipe(plist[i].fd) < 0)
			err(EXIT_FAILURE, "Failed to make a pipe");
	}

	for(int j=0; j<MAXPROCESSES; j++){
		pid[j]=fork();
		switch(pid[j]){
		case -1:
			err(EXIT_FAILURE, "Fork failed!");
		case 0:
			execcommand(p[j], plist, j);
		}
	}

	//El padre no usa los pipes -> los cerramos
	closepipes(plist, MAXPROCESSES-1);
	exitlastson=waitsons(pid);

	return exitlastson;

}

int main(int argc, char *argv[]){

	Processes process[MAXPROCESSES];
	int exitflag;

	if(argc!=MAXPROCESSES+1)
		err(EXIT_FAILURE, "usage: pipeline ['command arg...' 'command arg...' 'command arg...']");

	setprocesses(process, ++argv);
	exitflag=makeprocesses(process);

	exit(exitflag);

}
