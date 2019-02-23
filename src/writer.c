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
	struct s_mat * matStruct;

	if ((matStruct = (struct s_mat *) malloc(sizeof(struct s_mat))) == NULL) {
		perror("malloc");
		return NULL;
	}

	// init
	char * end;
	char buf[256];
	int nbMult;

	// lecture du nombre de multiplications de matrices
	printf("Nombre de multiplications de matrices :\n");
	while (fgets(buf, sizeof(buf), stdin)) {
		nbMult = strtol(buf, &end, 10);

		if (end == buf || * end != '\n') {
			printf("Rentrez un entier valide :\n");
		} else {
			break;
		}
	}
	
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
	for (i = 0; i < matStruct->nbMult; i += 2) {
		int j;
		// lecture des tailles
		for (j = 0; j < 2; j++) {
			printf("Taille de la matrice %d (<nbLignes> <nbColonnes>) :\n", i + j);
			while (fgets(buf, sizeof(buf), stdin)) {
				matStruct->matSize[i + j][0] = strtol(buf, &end, 10);
				matStruct->matSize[i + j][1] = strtol(end, &end, 10);

				if (end == buf || * end != '\n') {
					printf("Rentrez un format valide :\n");
				} else {
					break;
				}
			}
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
		for (j = 0; j < 2; j++) {
			int ligne, colonne;
			for (ligne = 0; ligne < matStruct->matSize[i + j][0]; ligne++) {
				printf("Ligne %d de la matrice %d (<val1,1 val1,2 val1,3 ...) :\n", ligne, i + j);
				while (fgets(buf, sizeof(buf), stdin)) {
					end = buf;
					for (colonne = 0; colonne < matStruct->matSize[i + j][1]; colonne++) {
						matStruct->matTab[i + j][ligne][colonne] = strtol(end, &end, 10);
					}

					if (end == buf || * end != '\n') {
						printf("Rentrez un format valide :\n");
					} else {
						break;
					}
				}
			}
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
