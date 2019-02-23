#ifndef _WRITER_
#define _WRITER_
#include "prodmat.h"

// création du fichier de données
// si le fichier existe, il est vidé
// retourne :
//	* le descripteur de fichier
//	* -1 en cas d'échec
int fileCreate(char * fileName);

// écriture des données dans le fichier
void dataWrite(int fd, struct s_mat * matStruct);

#endif
