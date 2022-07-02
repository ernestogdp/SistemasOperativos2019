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

int readlines(Path listpaths[], char *file, int numpath){

	FILE *f;
	char *line;
	char buffer[SIZE];

	if(access(file, F_OK | R_OK) == -1){
		//fprintf(stderr, "El fichero '%s' no existe o no es accesible\n", listpaths[i]);
		return -1;
	}
	if((f=fopen(file, "r")) == NULL)
		err(EXIT_FAILURE, "Failed to open %s", file);

	do{
		line=fgets(buffer, SIZE, f);
		if(line==NULL)
			break;
		buffer[strlen(buffer)-1]='\0';
		//Comprobamos que el fichero no este vacio
		if(buffer[0]=='\0')
			break;
		if(access(buffer, F_OK) == -1){
			//fprintf(stderr, "El path '%s' no existe\n", buffer);
			return -1;
		}
		if(setline(listpaths, buffer, numpath))
			numpath++;
	}while(numpath <= SIZE);

	fclose(f);

	return numpath;

}

int findrepeatedpath(Path plist[], int numpaths){
	int mostrepeatedpath=0;

	for(int z=0; z<numpaths; z++){
		/*
		 *  En caso de tener paths con mismo numero de apariciones,
		 *  se escoge el primero que aparezca.
		 */
		if(plist[z].count > plist[mostrepeatedpath].count){
			mostrepeatedpath=z;
		}
	}

	return mostrepeatedpath;

}

char * findcommonpath(int numfich, char *fich[], int *numpathsfind){

	struct Path paths[SIZE];
	int commonp=0;
	char *path;

	for(int j=0; j<numfich; j++){
		if((*numpathsfind =+ readlines(paths, fich[j], *numpathsfind)) == 0) {
			return NULL;
		}
	}

	commonp=findrepeatedpath(paths, *numpathsfind);
	path=paths[commonp].spath;

	return path;

}

void sonwork(char *f, int numoflines){
	int fdout=1;

	if(numoflines==-1)
		fdout=2;

	if(f != NULL && numoflines > 0){
		if((fdout=open(f, O_TRUNC | O_WRONLY | O_APPEND, 0777)) == -1)
			err(EXIT_FAILURE, "Failed to open %s", f);
	}

	dup2(fdout, 1);

	if(f != NULL)
		close(fdout);

	execl("/bin/ps", "ps", NULL);

}

void processps(char *file, int npaths){
	pid_t pid, wpid;
	int sts;

	pid=fork();
	switch(pid){
	case -1:
		err(EXIT_FAILURE, "fork failed!");
	case 0:
		sonwork(file, npaths);
		err(EXIT_FAILURE, "error on exit son");
	}

	while((wpid=wait(&sts)) != -1){
		if(WIFEXITED(sts) && WEXITSTATUS(sts)==-1){
			printf("Error on exit son\n");
		}
	}
}

int main(int argc, char * argv[]){
	char *commonpath=NULL;
	int numberofpaths=0;

	if (argc!=1)
		commonpath=findcommonpath(argc-1, argv+1, &numberofpaths);

	processps(commonpath, numberofpaths);

	exit(EXIT_SUCCESS);
}
