#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h> 		/* open */
#include <unistd.h> 	/* write, sysconf */
#include <signal.h> 	/* sigaction */
#include <sys/mman.h> 	/* mmap, mprotect */
#include <sys/stat.h> 	/* stat */
#include <sys/types.h> 	/* mprotect */
#include "writer.h"
#include "reader.h"
#include "prodmat.h"

int main(int argc, char * argv[]) {
	// depuis l'entrée standard
	s_mat * matStruct;
	int fd;
	int test;

	// lecture entrée utilisateur
	// écriture dans un fichier
	matStruct = dataRead(NULL);
	fd = fileCreate("coucou.txt");
	dataWrite(fd, matStruct);
	close(fd);
	free(matStruct);

	// depuis un fichier
	char * start;
	s_mat * newStruct;

	// projection mémoire du fichier
	start = fileMap("coucou.txt");
	newStruct = dataRead(start);
	free(newStruct);

	return EXIT_SUCCESS;
}
