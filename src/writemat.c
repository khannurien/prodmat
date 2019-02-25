#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>		/* close */
#include "reader.h"
#include "writer.h"
#include "prodmat.h"

int main(int argc, char * argv[]) {
	s_mat * matStruct;
	int fd;

	matStruct = dataRead(NULL);

	fd = fileCreate("test.txt");
	dataWrite(fd, matStruct);
	close(fd);
}