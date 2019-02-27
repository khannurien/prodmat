#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>		/* close */
#include "reader.h"
#include "writer.h"
#include "prodmat.h"

/**
 * @brief Programme de lecture des données d'une matrice depuis l'entrée standard.
 * Écrit la matrice sous forme textuelle dans le fichier dont le chemin est passé en paramètre.
 */
int main(int argc, char * argv[]) {
	s_mat * matStruct;
	char * fileName;
	int fd;

	// lecture du nom de fichier destination
	if (argc == 2) {
		fileName = argv[1];
	} else {
		printf("Usage: %s <fileName>\n", argv[0]);
	}

	// lecture des données de la matrice dans la structure matStruct
	matStruct = dataRead(NULL);

	// création du fichier résultat
	if ((fd = fileCreate(fileName)) == -1) {
		perror("fileCreate");
		exit(EXIT_FAILURE);
	}
	// écriture du fichier depuis les données de la structure
	dataWrite(fd, matStruct);

	// fin de l'écriture
	close(fd);
}