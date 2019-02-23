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

// création du fichier de données
// si le fichier existe, il est vidé
// retourne :
//	* le descripteur de fichier
//	* -1 en cas d'échec
int fileCreate(char * fileName) {
	int fd;

	if ((fd = open(fileName, O_CREAT|O_TRUNC|O_RDWR, 0666)) == -1) {
		perror("open");
		return -1;
	}

	return fd;
}

// écriture des données dans le fichier
void dataWrite(int fd, struct s_mat * matStruct) {
	int wr;
	char buf[256];

	// écriture du nombre de multiplications
	sprintf(buf, "%d\n", matStruct->nbMult);
	if ((wr = write(fd, buf, strlen(buf))) == -1) {
		perror("write");
		return;
	}

	// écriture des infos sur les matrices, deux à deux
	int i, j;
	for (i = 0; i <= matStruct->nbMult; i += 2) {
		for (j = 0; j < 2; j++) {
			// écriture des tailles
			sprintf(buf, "%d ", matStruct->matSize[i + j][0]);

			if ((wr = write(fd, buf, strlen(buf))) == -1) {
				perror("write");
				return;
			}

			sprintf(buf, "%d\n", matStruct->matSize[i + j][1]);

			if ((wr = write(fd, buf, strlen(buf))) == -1) {
				perror("write");
				return;
			}
		}

		for (j = 0; j < 2; j++) {
			// écriture des valeurs
			int ligne, colonne;
			for (ligne = 0; ligne < matStruct->matSize[i + j][0]; ligne++) {
				for (colonne = 0; colonne < matStruct->matSize[i + j][1]; colonne++) {
					sprintf(buf, "%d ", matStruct->matTab[i + j][ligne][colonne]);
					if ((wr = write(fd, buf, strlen(buf))) == -1) {
						perror("write");
						return;
					}
				}

				if ((wr = write(fd, "\n", sizeof(char))) == -1) {
					perror("write");
					return;
				}
			}
		}
	}

	// fermeture du descripteur de fichier
	close(fd);
}
