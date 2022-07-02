#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <err.h>
#include <fcntl.h>
#include <sys/wait.h>

enum{
		SIZE=512
};

int gettypefich(char *dotp, char *sext, char *pathfich, char *name){

	struct stat st;
	int result=0;


	if(stat(pathfich, &st) == -1)
		err(EXIT_FAILURE, "Failed to stat %s", pathfich);

	if(S_ISREG(st.st_mode)){
		if(dotp != NULL && strcmp(dotp+1, sext)==0){
			if(st.st_mode & (S_IRUSR | S_IRGRP | S_IROTH))
				result=1;
		}
	}

	if(S_ISDIR(st.st_mode) && strcmp(name, ".") != 0 && strcmp(name, "..") != 0){
		result = 2;
	}

	return result;
}

void changename(char * pdot, char *newp, char * oldp, char *fname, char *dir, char * finalext){

	pdot[1]='\0';
	sprintf(newp,"%s/%s%s", dir, fname, finalext);

	if(rename(oldp, newp) == 1)
		err(EXIT_FAILURE, "Failed to rename to %s", finalext);

}

void scrolldirectory(char *startextension, char *finalextension, char *directory){

	DIR *d;
	struct dirent *entry;
	char oldpath[SIZE];
	char newpath[SIZE];
	char fichname[SIZE];
	char *pointerdot;
	int typefich;


	if((d = opendir(directory))==NULL)
		err(EXIT_FAILURE, "Failed to open directory %s", directory);

	while((entry= readdir(d)) != NULL){

		sprintf(oldpath, "%s/%s", directory, entry->d_name);
		strncpy(fichname, entry->d_name, SIZE);
		pointerdot = strrchr(fichname, '.');

		typefich = gettypefich(pointerdot, startextension, oldpath, fichname);

		//Si es un archivo regular
		if(typefich == 1){
			changename(pointerdot, newpath, oldpath, fichname, directory, finalextension);

		//Si es un directorio
		}else if(typefich == 2){
			//printf("Cambiando de directorio a: %s\n", oldpath);
			scrolldirectory(startextension, finalextension, oldpath);
		}
	}

	if ((closedir(d)) == -1){
		err(EXIT_FAILURE, "Failed to close the directory");
	}
}

int main(int argc, char * argv[]){

	if (argc!=4)
		err(EXIT_FAILURE, "usage: chext [startextension finalextension directory]");

	scrolldirectory(argv[1], argv[2], argv[3]);

	exit(EXIT_SUCCESS);
}
