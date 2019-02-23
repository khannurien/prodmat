#include <stdlib.h>
#include <stdio.h>
#include <string.h>		/* strlen, strcat */
#include <fcntl.h>		/* open */
#include <unistd.h>		/* write, stat */
#include <sys/stat.h>	/* stat */
#include <sys/types.h>	/* stat */	
#include <sys/mman.h>	/* mmap */
#include "reader.h"
#include "prodmat.h"

// projection mémoire
// retourne :
// 	* un pointeur vers la zone de projection
// 	* NULL en cas d'échec
char * fileMap(char * fileName) {
	char * memPtr;
	int fd;
	struct stat fileStat;

	if ((fd = open(fileName, O_RDONLY)) == -1) {
		perror("open");
		return NULL;
	}

	if (stat(fileName, &fileStat) == -1) {
		perror("stat");
		return NULL;
	}

	if ((memPtr = mmap(NULL, (size_t) fileStat.st_size, PROT_READ, MAP_SHARED, fd, 0)) == MAP_FAILED) {
		perror("mmap");
		// switch (errno) ...
		return NULL;
	}

	close(fd);

	return memPtr;
}

// lecture des données des matrices depuis un pointeur
// retourne :
// 	* un pointeur vers la structure s_mat résultante
// 	* NULL en cas d'échec
struct s_mat * dataReadFromMem(char * start) {
	struct s_mat * matStruct;

	if ((matStruct = (struct s_mat *) malloc(sizeof(struct s_mat))) == NULL) {
		perror("malloc");
		return NULL;
	}

	// init
	char * end;
	int nbMult;

	// lecture du nombre de multiplications de matrices
	nbMult = strtol(start, &end, 10);
	
	// inscription dans la structure
	matStruct->nbMult = nbMult;		// nombre de multiplications
	matStruct->nbMat = nbMult * 2;	// nombre total de matrices
	// initialisation du tableau de tailles des matrices
	if ((matStruct->matSize = (int **) malloc(matStruct->nbMat * sizeof(int *))) == NULL) {
		perror("malloc");
		free(matStruct);
		return NULL;
	}

	int i;
	for (i = 0; i < matStruct->nbMat; i++) {
		if ((matStruct->matSize[i] = (int *) malloc(2 * sizeof(int))) == NULL) {
			perror("malloc");
			free(matStruct->matSize);
			free(matStruct);
			return NULL;
		}
	}

	// init du tableau de matrices
	if ((matStruct->matTab = (int ***) malloc(matStruct->nbMat * sizeof(int **))) == NULL) {
		perror("malloc");
		return NULL;
	}

	// lecture des données des nbMult * 2 matrices
	for (i = 0; i <= matStruct->nbMult; i += 2) {
		int j;
		// lecture des tailles
		for (j = 0; j < 2; j++) {
			matStruct->matSize[i + j][0] = strtol(end, &end, 10);
			matStruct->matSize[i + j][1] = strtol(end, &end, 10);
		}

		// init des lignes de chaque matrice
		int k;
		for (k = 0; k < matStruct->nbMat; k++) {
			if ((matStruct->matTab[k] = (int **) malloc(matStruct->matSize[k][0] * sizeof(int *))) == NULL) {
				perror("malloc");
				free(matStruct->matTab);
				return NULL;
			}
			// init des colonnes de chaque ligne
			int j;
			for (j = 0; j < matStruct->matSize[k][0]; j++) {
				if ((matStruct->matTab[k][j] = (int *) malloc(matStruct->matSize[k][1] * sizeof(int))) == NULL) {
					perror("malloc");
					free(matStruct->matTab);
					return NULL;
				}
			}
		}

		// lecture des données
		// TODO: vérifier que le format concorde...
		for (j = 0; j < 2; j++) {
			int ligne, colonne;
			for (ligne = 0; ligne < matStruct->matSize[i + j][0]; ligne++) {
				for (colonne = 0; colonne < matStruct->matSize[i + j][1]; colonne++) {
					matStruct->matTab[i + j][ligne][colonne] = strtol(end, &end, 10);
				}
			}
		}
	}

	return matStruct;
}