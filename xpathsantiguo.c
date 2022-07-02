#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <err.h>
#include <fcntl.h>
#include <sys/wait.h>

enum{
		SIZE=1024
};

struct Path{
	char spath[SIZE];
	int count;
};

typedef struct Path Path;

int setline(Path pathlist[], char *buf, int nump){

	int find=0;

	for(int i=0; i<nump; i++){
		if(strcmp(pathlist[i].spath, buf)==0){
			pathlist[i].count++;
			find=1;
			break;
		}
	}

	if(!find){
		strncpy(pathlist[nump].spath, buf, SIZE);
		pathlist[nump].count=1;
	}

	return !find;
}

char * findcommonpath(int numarg, char *arg[]){

	FILE *f[numarg];
	char *line;
	char buffer[SIZE];
	struct Path plist[SIZE];
	int numpath=0;
	int mostrepeatedpath=0;
	char *path;

	for(int i=0; i<numarg; i++){
		if(access(arg[i], F_OK | R_OK) == -1){
			//fprintf(stderr, "El fichero '%s' no existe o no es accesible\n", arg[i]);
			return NULL;
		}
		if((f[i]=fopen(arg[i], "r")) == NULL)
			err(EXIT_FAILURE, "Failed to open %s", arg[i]);
	}

	for(int j=0; j<numarg; j++){
		do{
			line=fgets(buffer, SIZE, f[j]);
			if(line==NULL)
				break;
			buffer[strlen(buffer)-1]='\0';
			if(access(buffer, F_OK) == -1){
				//fprintf(stderr, "El path '%s' no existe\n", buffer);
				return NULL;
			}
			if(setline(plist, buffer, numpath))
				numpath++;
		}while(line != NULL || numpath>=SIZE);
	}

	for(int z=0; z<numpath; z++){
		/*
		 *  En caso de tener paths con mismo numero de apariciones,
		 *  se escoge el primero que aparezca.
		 */
		if(plist[z].count > plist[mostrepeatedpath].count){
			mostrepeatedpath=z;
		}
	}
	path=plist[mostrepeatedpath].spath;
	return path;

}

void sonwork(char *f){
	int writefile;
	if(f != NULL){
		if((writefile=open(f, O_TRUNC | O_WRONLY | O_APPEND, 0777)) == -1)
			err(EXIT_FAILURE, "Failed to open %s", f);
		dup2(writefile, 1);
		close(writefile);
	}else{
		dup2(2, 1);
	}
	execl("/bin/ps", "ps", NULL);

}

void processps(char *file){
	pid_t pid, wpid;
	int sts;

	pid=fork();
	switch(pid){
	case -1:
		err(EXIT_FAILURE, "fork failed!");
	case 0:
		sonwork(file);
		err(EXIT_FAILURE, "error on exit son");
	}

	while((wpid=wait(&sts)) != -1){
		if(WIFEXITED(sts)){
			if(WEXITSTATUS(sts)==-1){
				printf("Error on exit son\n");
			}
		}
	}
}


int main(int argc, char * argv[]){
	char *commonpath=NULL;

	//findcommonpath: devuelve el path mas común. NULL en caso de error
	if (argc!=1)
		commonpath=findcommonpath(--argc, ++argv);

	processps(commonpath);

	exit(EXIT_SUCCESS);
}
