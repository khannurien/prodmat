#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>		/* open */
#include <unistd.h>		/* write, stat */
#include <sys/stat.h>	/* stat */
#include <sys/types.h>	/* stat */	
#include <sys/mman.h>	/* mmap */
#include "writer.h"
#include "prodmat.h"

// création du fichier de données
// si le fichier existe, il est vidé
// retourne :
//	* le descripteur de fichier
//	* -1 en cas d'échec
int fileCreate(char * fileName) {
	int fd;

	if ((fd = open(fileName, O_CREAT|O_TRUNC|O_RDWR)) == -1) {
		perror("open");
		return -1;
	}

	return fd;
}

// lecture de l'entrée utilisateur
// retourne :
// 	* un pointeur vers la structure s_mat
// 	* NULL en cas d'échec
struct s_mat * dataRead(void) {
	int i, j;
	struct s_mat * matStruct;

	if ((matStruct = (struct s_mat *) malloc(sizeof(struct s_mat))) == NULL) {
		perror("malloc");
		return NULL;
	}

	// init
	matStruct->nbMult = 0;
	matStruct->nbMat = 0;

	if ((matStruct->matSize = (int *) malloc(matStruct->nbMat * sizeof(int))) == NULL) {
		perror("malloc");
		return NULL;
	}

	if ((matStruct->matTab = (int **) malloc(matStruct->nbMat * sizeof(int *))) == NULL) {
		perror("malloc");
		return NULL;
	}

	for (i = 0; i < matStruct->nbMat; i++) {
		matStruct->matSize[i] = 0;

		if ((matStruct->matTab[i] = (int *) malloc(matStruct->matSize[i] * sizeof(int))) == NULL) {
			perror("malloc");
			return NULL;
		}

		for (j = 0; j < matStruct->matSize[i]; j++) {
			matStruct->matTab[i][j] = 0;
		}
	}

	return matStruct;
}

// écriture des données dans le fichier
// retourne :
// 	* le nombre d'octets écrits
// 	* -1 en cas d'échec
int dataWrite(int fd, struct s_mat * matStruct) {
	int nbWr;

	return nbWr;
}

// projection mémoire
// retourne :
// 	* un pointeur vers la zone de projection
// 	* NULL en cas d'échec
char * fileMap(int fd, char * fileName) {
	char * file;
	struct stat fileStat;

	if (stat(fileName, &fileStat) == -1) {
		perror("stat");
		return NULL;
	}

    if ((file = mmap(NULL, (size_t) fileStat.st_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0)) == MAP_FAILED) {
		perror("mmap");
		// switch (errno) ...
		return NULL;
	}

	return file;
}
