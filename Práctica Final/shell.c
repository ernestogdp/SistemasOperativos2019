//gcc -c -Wall -Wshadow -g shell.c
//gcc -o shell shell.o

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
	SIZE=1024,
	MAXNUMARG=10,
	MAXNUMPROCESSES=20
};

typedef struct Processes Processes;
typedef struct Pipelist Pipelist;

struct Pipelist{
	int fd[2];
};

struct Processes{
	char *command;
	char *arg[MAXNUMARG];
	int numarg;
};

Processes process[MAXNUMPROCESSES];

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

int extractredir(char *buffer, char *file, char *redir)
{

	char *parts[SIZE];
	int numparts=0;

	numparts=extractwords(buffer, redir, parts);
	if (numparts>1){
		extractwords(parts[1], " ", parts);
		strcpy(file, parts[0]);
		return 1;
	}else{
		return 0;
	}
}

int translatevar(char **variables, int numvar)
{
	char buffer[SIZE/2];
	char *auxbuf[SIZE/2];
	char *valor;
	int find=0;

	for (int i=0; i<numvar; i++){
		if(strchr(variables[i], '$')){
			strcpy(buffer, variables[i]);
			extractwords(buffer, "$", auxbuf);
			if((valor=getenv(auxbuf[0]))!=NULL){
				variables[i]= valor;
				find=1;
			}else{
				printf("Error: var %s does not exist\n", auxbuf[0]);
				find=-1;
			}
		}
	}
	return find;
}

int makearraycommands(char * buf, char *fin, char *fout, int *flagin, int *flagout)
{
	char *line1[SIZE];
	char *line2[SIZE];
	char *words[SIZE];
	char copybuffer1[SIZE];
	char copybuffer2[SIZE];
	int numprocess;
	int numwords;
	//int j=0;

	strcpy(copybuffer1, buf);
	strcpy(copybuffer2, buf);

	extractwords(copybuffer1, "\n\r", line1);
	extractwords(copybuffer2, "\n\r", line2);

	*flagin=extractredir(line2[0], fin, "<");
	*flagout=extractredir(line1[0], fout, ">");

	strtok(buf, "<>");
	numprocess = extractwords(buf, "|\t\r\n", words);
	//printf("Numero de procesos: %d\n", numprocess);
	for (int i=0; i<numprocess; i++){
		numwords=extractwords(words[i]," ", process[i].arg);
		if(translatevar(process[i].arg, numwords)==-1){
			numprocess=-1;
			break;
		}
		process[i].command=process[i].arg[0];
		//retroceder una posicion el array de argumentos
		for (int z=0; z<numwords; z++){
		  	process[i].arg[z]=process[i].arg[z+1];
		}
		process[i].arg[numwords-1]=NULL;
		process[i].numarg = numwords-1;
		//Comprobararraydecomandos y redireccionamiento
		/*printf("==================\n");
		printf("comando: %s\n", process[i].command);
		printf("------------------\n");
		j=0;
		while(process[i].arg[j] != NULL){
		  	printf("Argumento %d: %s\n", j, process[i].arg[j]);
			j++;
		}
		printf("Numero de argumentos: %d\n", process[i].numarg);*/
	}
	return numprocess;
}

void changedirectory(Processes proceso)
{
	char *home;

	if(proceso.numarg){
		if(chdir(proceso.arg[0])==-1){
			printf("%s\n", strerror(errno));
		}
	}else{
		home=getenv("HOME");
		chdir(home);
	}
}

void redireccionar(char *file, int descripterfich)
{
	int fp;

	if(descripterfich>2)
		err(EXIT_FAILURE, "Descripter fich must be 0, 1, or 2");

	if(descripterfich==0){
		fp=open(file, O_RDONLY);
	} else {
		fp=open(file, O_WRONLY|O_CREAT|O_TRUNC,0664);
	}
	dup2(fp, descripterfich);
	close(fp);
}

int makeexecutable(char *ex, char *cmd)
{
	char *path;
	char completepath[SIZE];
	char workdir[SIZE];
	char *ejecpath[SIZE];
	int numexecpath;
	int find=0;

	path=getenv("PATH");
	getcwd(workdir,SIZE);
	//prioridad del directorio de trabajo ante PATH
	sprintf(completepath, "%s:%s", workdir, path);
	numexecpath = extractwords(completepath, ":", ejecpath);

	for(int i=0; i<numexecpath; i++){
		sprintf(ex, "%s/%s", ejecpath[i], cmd);
		if(access(ex, X_OK) == 0){
			find=1;
			//printf("El binario se encuentra en: %s\n", ex);
			break;
		}
	}

	if(!find)
		printf("El comando %s no se ha encontrado\n", cmd);
	return find;

}

void execexpandarguments(Processes proc, char *exe)
{
	glob_t g;
	int isfirstglob=1;
	int i;

	g.gl_offs = 2;
	for(int j=0; j<proc.numarg; j++){
		if(strchr(proc.arg[j], '*')!=NULL){
			if(isfirstglob){
				glob (proc.arg[j], GLOB_DOOFFS, NULL, & g);
				isfirstglob=0;
			}else{
				glob(proc.arg[j], GLOB_DOOFFS | GLOB_APPEND, NULL, & g);
			}
		}
	}
	g.gl_pathv[0] = proc.command;
	for(i=1; i<proc.numarg+1; i++){
		if(strchr(proc.arg[i-1], '*')==NULL){
				g.gl_pathv[i]=proc.arg[i-1];
			}
	}
	execv(exe, g.gl_pathv);
	err(EXIT_FAILURE, "execv failed!");
}

void execcomand(Processes p, Pipelist pipe[], int numcommand, int maxcommand,
						char *in, char *out, int existin, int existout)
{
	char executable[SIZE];
	char *arrayaux[SIZE];
	int expand=0;

	if(!makeexecutable(executable, p.command))
		err(EXIT_FAILURE, "aborted thread");

	//Imprimir el path ejecutable
	//printf("El binario del comando se encuentra en: %s\n", executable);
	if(maxcommand>1){
		if(numcommand==1){
			if(existin){
				redireccionar(in, 0);
			}
			dup2(pipe[0].fd[1], 1);
		}else if(numcommand==maxcommand){
			dup2(pipe[numcommand-2].fd[0], 0);
			if(existout){
				redireccionar(out, 1);
			}
		}else{
			dup2(pipe[numcommand-2].fd[0], 0);
			dup2(pipe[numcommand-1].fd[1], 1);
		}
		for(int j=0; j<maxcommand-1; j++){
			if(close(pipe[j].fd[0]) == -1)
				err(EXIT_FAILURE, "Failed to close");
			if(close(pipe[j].fd[1]) == -1)
				err(EXIT_FAILURE, "Failed to close");
		}
	}else{
		if(existin){
			redireccionar(in, 0);
		}
		if(existout){
			redireccionar(out, 1);
		}
	}

	if(p.arg[0]==NULL){
		execl(executable, "program", NULL);
		err(EXIT_FAILURE, "execl failed!");
	}else{
		for(int j=0; j<p.numarg; j++){
			if(strchr(p.arg[j], '*')!=NULL){
				expand=1;
			}
		}
		if(expand){
			execexpandarguments(p, executable);
		}else{
			arrayaux[0]=p.command;
			for(int z=1; z<p.numarg+1; z++){
				arrayaux[z]=p.arg[z-1];
			}
			execv(executable, arrayaux);
			err(EXIT_FAILURE, "%s failed!", p.command);
		}
	}
}

int waitson()
{
	int sts;
	pid_t wpid;
	int exitflag = 1;

	while((wpid= wait(&sts)) != -1){
		if(WIFEXITED(sts)){
			if (WEXITSTATUS(sts)<0){
				exitflag = 0;
			}
		}
	}
	return exitflag;
}

void docommands(char *fin, char *fout, int flagin, int flagout, int numprocesses, int and)
{
	Pipelist plist[numprocesses];
	pid_t pid;
	char rubish[]="/dev/null";

	if(numprocesses>1){
		for(int z=0;z<numprocesses-1;z++){
			if(pipe(plist[z].fd) < 0)
				err(EXIT_FAILURE, "Failed to make a pipe");
	  	}
	}

	for (int i=0; i<numprocesses; i++){

		pid=fork();
		switch(pid){
		case -1:
			err(EXIT_FAILURE, "fork failed!");
		case 0:
			if(i==0 && and==1 && flagin == 0){
				redireccionar(rubish, 0);
			}
			execcomand(process[i], plist, i+1, numprocesses, fin, fout, flagin, flagout);
		}
	}
	if(numprocesses>1){
		for(int j=0; j<numprocesses-1; j++){
			if(close(plist[j].fd[0]) == -1)
				err(EXIT_FAILURE, "Failed to close");
			if(close(plist[j].fd[1]) == -1)
				err(EXIT_FAILURE, "Failed to close");
		}
	}
	if(!and){
		if (waitson() == 0)
			err(EXIT_FAILURE, "son failed!");
	}

}

void savevar(char *entry)
{
	char *var[SIZE/2];
	int numvar = 2;

	if(extractwords(entry, "=", var) != numvar){
		printf("Introduzca -> #variable = #valor ");
	}else{
		strtok(var[1], "\n");
		/*  setenv -> tercer argumento=1 sobreescribe
		 	el valor de la variable si existe  */
		if(setenv(var[0], var[1], 1) == -1)
			err(EXIT_FAILURE, "setenv failed!");
	}

}

int checkand(char *line)
{
	char *partsline[SIZE];

	if(extractwords(line, "&", partsline)>1){
		if(strcmp(partsline[1],"\n")==0){
			return 1;
		}else{
			printf("enter the symbol (&) at the end of the line\n");
			return -1;
		}
	}else{
		return 0;
	}
}

int main(int argc, char* argv[])
{
	char *entry;
	char workdirectory[SIZE];
	char buffer[SIZE];
	char fichin[SIZE];
	char fichout[SIZE];
	int numprocesses;
	int existin=0;
	int existout=0;
	char *user;
	int containsand=0;

	while(1){
		user = getenv("USER");
		getcwd(workdirectory, SIZE);
		printf("%s@ernestterminal:~%s$ ", user, workdirectory);
		entry=fgets(buffer, SIZE, stdin);
		if(strcmp(buffer,"exit\n") == 0 || entry == NULL){
			break;
		}
		if(strchr(entry, '=')){
			savevar(entry);
			continue;
		}
 		if(strcmp(buffer,"\n") != 0){
			containsand=checkand(entry);
			numprocesses=makearraycommands(buffer, fichin, fichout, &existin, &existout);
			if(numprocesses!=-1 && containsand != -1){
				if(strcmp(process[0].command, "cd") == 0){
					changedirectory(process[0]);
				}else{
					docommands(fichin, fichout, existin, existout, numprocesses, containsand);
				}
			}
		}
	}
	exit(EXIT_SUCCESS);
}
