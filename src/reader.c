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

/**
 * Projection mémoire pour un fichier désigné par son chemin.
 * Retourne un pointeur vers la zone de projection, ou NULL en cas d'échec.
 */
char * fileMap(char * fileName) {
	char * memPtr;
	int fd;
	struct stat fileStat;

	// ouverture du fichier en lecture seule
	if ((fd = open(fileName, O_RDONLY)) == -1) {
		perror("open");
		return NULL;
	}

	// obtention de sa taille (notamment) dans la structure stat
	if (stat(fileName, &fileStat) == -1) {
		perror("stat");
		return NULL;
	}

	// projection du fichier dans une zone mémoire partagée en lecture seule
	if ((memPtr = mmap(NULL, (size_t) fileStat.st_size, PROT_READ, MAP_SHARED, fd, 0)) == MAP_FAILED) {
		perror("mmap");
		// switch (errno) ...
		return NULL;
	}

	// fermeture du descripteur de fichier
	close(fd);

	return memPtr;
}

struct s_mat * dataRead(char * start) {
	struct s_mat * matStruct;

	if ((matStruct = (struct s_mat *) malloc(sizeof(struct s_mat))) == NULL) {
		perror("malloc");
		return NULL;
	}

	// init
	char * end;
	char buf[256]; // en cas de lecture depuis stdin
	int nbMult;

	// lecture du nombre de multiplications de matrices
	if (start == NULL) {
		// stdin
		printf("Nombre de multiplications de matrices :\n");
		while (fgets(buf, sizeof(buf), stdin)) {
			nbMult = strtol(buf, &end, 10);

			if (end == buf || * end != '\n') {
				printf("Rentrez un entier valide :\n");
			} else {
				break;
			}
		}
	} else {
		// mmap
		nbMult = strtol(start, &end, 10);
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
	for (i = 0; i <= matStruct->nbMult; i += 2) {
		int j;
		// lecture des tailles
		for (j = 0; j < 2; j++) {
			if (start == NULL) {
				// stdin
				printf("Taille de la matrice %d (<nbLignes> <nbColonnes>) :\n", i + j);
				while (fgets(buf, sizeof(buf), stdin)) {
					matStruct->matSize[i + j][0] = strtol(buf, &end, 10);
					matStruct->matSize[i + j][1] = strtol(end, &end, 10);

					if (end == buf || * end != '\n' || matStruct->matSize[i + j][0] == 0
													|| matStruct->matSize[i + j][1] == 0) {
						printf("Rentrez un format valide :\n");
					} else {
						break;
					}
				}
			} else {
				// mmap
				matStruct->matSize[i + j][0] = strtol(end, &end, 10);
				matStruct->matSize[i + j][1] = strtol(end, &end, 10);	
			}
		}

		// allocation des lignes de chaque matrice
		int k;
		for (k = 0; k < matStruct->nbMat; k++) {
			if ((matStruct->matTab[k] = (int **) malloc(matStruct->matSize[k][0] * sizeof(int *))) == NULL) {
				perror("malloc");
				free(matStruct->matTab);
				return NULL;
			}
			// allocation des colonnes de chaque ligne
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
				if (start == NULL) {
					// stdin
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
				} else {
					// mmap
					for (colonne = 0; colonne < matStruct->matSize[i + j][1]; colonne++) {
						matStruct->matTab[i + j][ligne][colonne] = strtol(end, &end, 10);
					}					
				}
			}
		}
	}

	return matStruct;
}