#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <err.h>
#include <fcntl.h>

enum{
		SIZE=1024
};

void processcopy(int fread, int fwrite, int limit){

	char buf[SIZE];
	int bytesread;
	int bytestowrite;
	int writeall=0;

	while((bytesread = read(fread, buf, sizeof(buf))) > 0 && !writeall){
	   	if(bytesread > limit){
		   	bytestowrite = limit;
			writeall=1;
	   	}else{
		   	bytestowrite = bytesread;
		   	limit -=bytesread;
	   	}
		if (write(fwrite, buf, bytestowrite) != bytestowrite)
        	err(EXIT_FAILURE, "Failed to write");
	}
	if(bytesread ==-1)
		err(EXIT_FAILURE, "Failed to read");

}

void copyfile(char *arg[], int existfread, int existfwrite){

	int readfile=0; //Por defecto, la entrada estandar
	int writefile=1; //Por defecto, la salida estandar
	int limittowrite=atoi(arg[3]);

	if(existfread){
		if((readfile=open(arg[0], O_RDONLY)) == -1)
			err(EXIT_FAILURE, "Failed to open %s", arg[0]);
		if(lseek(readfile, atoi(arg[2]), SEEK_SET) == -1)
			err(EXIT_FAILURE, "Failed to change the offset of %s %s positions", arg[0], arg[2]);
	}
	if(existfwrite)
		if((writefile=open(arg[1], O_CREAT | O_TRUNC | O_WRONLY | O_APPEND, 0640)) == -1)
			err(EXIT_FAILURE, "Failed to open %s", arg[1]);

	processcopy(readfile, writefile, limittowrite);

   	if ((close(readfile)) == -1)
	   	err(EXIT_FAILURE, "Failed to close read file");
	if ((close(writefile)) == -1)
	   	err(EXIT_FAILURE, "Failed to close write file");

}

int main(int argc, char * argv[]){

	int existreadfile=1;
	int existwritefile=1;

	if (argc != 5){
        err(EXIT_FAILURE, "usage: copybytes [srcfile outfile offset bytestocopy]");
    }
	if(strcmp(argv[1], "-") == 0)
		existreadfile=0;
	if(strcmp(argv[2], "-") == 0)
		existwritefile=0;

	copyfile(++argv, existreadfile, existwritefile);

	exit(EXIT_SUCCESS);
}
