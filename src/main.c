#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h> 		/* open */
#include <unistd.h> 	/* write, sysconf */
#include <signal.h> 	/* sigaction */
#include <sys/mman.h> 	/* mmap, mprotect */
#include <sys/stat.h> 	/* stat */
#include <sys/types.h> 	/* mprotect */
#include "writer.h"
#include "prodmat.h"

int main(int argc, char * argv[]) {
	s_mat * matStruct;
	int fd;
	int test;

	matStruct = dataRead();
	fd = fileCreate("coucou.txt");
	dataWrite(fd, matStruct);

	return EXIT_SUCCESS;
}
