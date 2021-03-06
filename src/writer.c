#include <stdlib.h>
#include <stdio.h>
#include <string.h>		/* strlen, strcat */
#include <fcntl.h>		/* open */
#include <unistd.h>		/* write, stat */
#include <sys/stat.h>	/* stat */
#include <sys/types.h>	/* stat */	
#include <sys/mman.h>	/* mmap */
#include "writer.h"
#include "prodmat.h"

/**
 * @brief Création d'un fichier de données désigné par son chemin.
 * Si le fichier existe, il est vidé.
 * La fonction retourne le descripteur de fichier, ou -1 en cas d'échec.
 * 
 * @param fileName Le chemin du fichier à créer.
 * @return int Le descripteur de fichier associé.
 */
int fileCreate(char * fileName) {
	int fd;

	// ouverture du fichier en lecture et écriture -- créé s'il n'existe pas, vidé s'il existe
	if ((fd = open(fileName, O_CREAT|O_TRUNC|O_RDWR, 0666)) == -1) {
		perror("open");
		return -1;
	}

	return fd;
}

/**
 * @brief Écriture des données dans un fichier désigné par son descripteur.
 * 
 * @param fd Le descripteur du fichier dans lequel écrire.
 * @param matStruct La structure contenant les données de la matrice
 */
void dataWrite(int fd, struct s_mat * matStruct) {
	int wr;
	char buf[8];

	// écriture du nombre de multiplications
	sprintf(buf, "%d\n", matStruct->nbMult);
	if ((wr = write(fd, buf, strlen(buf))) == -1) {
		perror("write");
		exit(EXIT_FAILURE);
	}

	// écriture des infos sur les matrices, deux à deux
	int i, j;
	for (i = 0; i < matStruct->nbMult; i++) {
		for (j = 0; j < 2; j++) {
			// écriture des tailles
			sprintf(buf, "%d ", matStruct->matSize[i * 2 + j][0]);

			if ((wr = write(fd, buf, strlen(buf))) == -1) {
				perror("write");
				exit(EXIT_FAILURE);
			}

			sprintf(buf, "%d\n", matStruct->matSize[i * 2 + j][1]);

			if ((wr = write(fd, buf, strlen(buf))) == -1) {
				perror("write");
				exit(EXIT_FAILURE);
			}
		}

		for (j = 0; j < 2; j++) {
			// écriture des valeurs
			int ligne, colonne;
			for (ligne = 0; ligne < matStruct->matSize[i * 2 + j][0]; ligne++) {
				for (colonne = 0; colonne < matStruct->matSize[i * 2 + j][1]; colonne++) {
					sprintf(buf, "%d ", matStruct->matTab[i * 2 + j][ligne][colonne]);
					if ((wr = write(fd, buf, strlen(buf))) == -1) {
						perror("write");
						exit(EXIT_FAILURE);
					}
				}

				if ((wr = write(fd, "\n", sizeof(char))) == -1) {
					perror("write");
					exit(EXIT_FAILURE);
				}
			}
		}
	}

	// fermeture du descripteur de fichier
	close(fd);
}

/**
 * @brief Écriture du résultat d'un produit matriciel dans un fichier.
 * 
 * @param fd Le descripteur du fichier dans lequel écrire.
 * @param matrix La matrice de résultat à écrire.
 * @param maxL Le nombre de lignes dans la matrice résultat.
 * @param maxC Le nombre de colonnes dans la matrice résultat.
 */
void resWrite(int fd, int ** matrix, int maxL, int maxC) {
	int i, j;
	char buf[8];

	for (i = 0; i < maxL; i++) {
		for (j = 0; j < maxC; j++) {
			sprintf(buf, "%d ", matrix[i][j]);
			if ((write(fd, buf, strlen(buf))) == -1) {
				perror("write");
				exit(EXIT_FAILURE);
			}
		}

		if ((write(fd, "\n", sizeof(char))) == -1) {
			perror("write");
			exit(EXIT_FAILURE);
		}
	}

	if ((write(fd, "\n", sizeof(char))) == -1) {
		perror("write");
		exit(EXIT_FAILURE);
	}
}
