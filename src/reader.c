#include <stdlib.h>
#include <stdio.h>
#include <string.h>		/* strlen, strcat */
#include <fcntl.h>		/* open */
#include <unistd.h>		/* write, stat */
#include <sys/stat.h>	/* stat */
#include <sys/types.h>	/* stat */	
#include <sys/mman.h>	/* mmap */
#include <errno.h>		/* errno */
#include <limits.h>		/* LONG_MAX, LONG_MIN, ERANGE */
#include "reader.h"
#include "prodmat.h"

/**
 * @brief Projection mémoire pour un fichier désigné par son chemin.
 * 
 * @param fileName Le fichier depuis lequel effectuer la projection.
 * @return char* Un pointeur vers le début de la zone mémoire de projection (NULL en cas d'échec).
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

/**
 * @brief Lecture des données des matrices depuis l'entrée standard ou depuis un pointeur vers le début d'une zone de projection mémoire.
 * 
 * @param start Le pointeur vers la zone de projection, ou NULL si l'on veut lire depuis l'entrée standard.
 * @return struct s_mat* Un pointeur vers la structure résultante (NULL en cas d'échec).
 */
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
			// test strtol
			errno = 0;
			nbMult = strtol(buf, &end, 10);
			if ((nbMult == LONG_MAX || nbMult == LONG_MIN) && errno == ERANGE) {
				perror("strtol");
				return NULL;
			}

			// test format
			if (end == buf || * end != '\n') {
				printf("Rentrez un entier valide :\n");
			} else {
				break;
			}
		}
	} else {
		// mmap
		errno = 0;
		nbMult = strtol(start, &end, 10);
		if ((nbMult == LONG_MAX || nbMult == LONG_MIN) && errno == ERANGE) {
			perror("strtol");
			return NULL;
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
		for (i = 0; i < matStruct->nbMat; i++) {
			free(matStruct->matSize[i]);
		}
		free(matStruct->matSize);
		free(matStruct);
		return NULL;
	}

	// lecture des données des nbMult * 2 matrices
	for (i = 0; i < matStruct->nbMat; i += 2) {
		int j;
		// lecture des tailles
		for (j = 0; j < 2; j++) {
			if (start == NULL) {
				// stdin
				printf("Taille de la matrice %d (<nbLignes> <nbColonnes>) :\n", i + j);
				while (fgets(buf, sizeof(buf), stdin)) {
					// test strtol
					errno = 0;
					matStruct->matSize[i + j][0] = strtol(buf, &end, 10);
					if ((matStruct->matSize[i + j][0] == LONG_MAX || matStruct->matSize[i + j][0] == LONG_MIN) && errno == ERANGE) {
						perror("strtol");
						return NULL;
					}

					// test strtol
					errno = 0;
					matStruct->matSize[i + j][1] = strtol(end, &end, 10);
					if ((matStruct->matSize[i + j][0] == LONG_MAX || matStruct->matSize[i + j][0] == LONG_MIN) && errno == ERANGE) {
						perror("strtol");
						return NULL;
					}

					if (end == buf || * end != '\n' || matStruct->matSize[i + j][0] == 0
													|| matStruct->matSize[i + j][1] == 0) {
						printf("Rentrez un format valide :\n");
					} else {
						break;
					}
				}
			} else {
				// mmap
				// test strtol
				errno = 0;
				matStruct->matSize[i + j][0] = strtol(end, &end, 10);
				if ((matStruct->matSize[i + j][0] == LONG_MAX || matStruct->matSize[i + j][0] == LONG_MIN) && errno == ERANGE) {
					perror("strtol");
					return NULL;
				}

				// test strtol
				errno = 0;
				matStruct->matSize[i + j][1] = strtol(end, &end, 10);	
				if ((matStruct->matSize[i + j][0] == LONG_MAX || matStruct->matSize[i + j][0] == LONG_MIN) && errno == ERANGE) {
					perror("strtol");
					return NULL;
				}
			}
		}

		// allocation des lignes de chaque matrice
		for (j = 0; j < 2; j++) {
			if ((matStruct->matTab[i + j] = (int **) malloc(matStruct->matSize[i + j][0] * sizeof(int *))) == NULL) {
				perror("malloc");
				for (i = 0; i < matStruct->nbMat; i++) {
					free(matStruct->matSize[i]);
				}
				free(matStruct->matSize);
				free(matStruct->matTab);
				free(matStruct);
				return NULL;
			}
			// allocation des colonnes de chaque ligne
			int k;
			for (k = 0; k < matStruct->matSize[i + j][0]; k++) {
				if ((matStruct->matTab[i + j][k] = (int *) malloc(matStruct->matSize[i + j][1] * sizeof(int))) == NULL) {
					perror("malloc");
					for (i = 0; i < matStruct->nbMat; i++) {
						free(matStruct->matSize[i]);
						free(matStruct->matTab[i]);
					}
					free(matStruct->matSize);
					free(matStruct->matTab);
					free(matStruct);
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
					printf("Ligne %d de la matrice %d (<val1,1> <val1,2> <val1,3> ...) :\n", ligne, i + j);
					while (fgets(buf, sizeof(buf), stdin)) {
						end = buf;
						for (colonne = 0; colonne < matStruct->matSize[i + j][1]; colonne++) {
							// test strtol
							errno = 0;
							matStruct->matTab[i + j][ligne][colonne] = strtol(end, &end, 10);
							if ((matStruct->matTab[i + j][ligne][colonne] == LONG_MAX || matStruct->matTab[ligne][colonne] == (int) LONG_MIN) && errno == ERANGE) {
								perror("strtol");
								return NULL;
							}
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
						// test strtol
						errno = 0;
						matStruct->matTab[i + j][ligne][colonne] = strtol(end, &end, 10);
						if ((matStruct->matTab[i + j][ligne][colonne] == LONG_MAX || matStruct->matTab[ligne][colonne] == (int) LONG_MIN) && errno == ERANGE) {
							perror("strtol");
							return NULL;
						}
					}					
				}
			}
		}
	}

	return matStruct;
}
