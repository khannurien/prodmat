#ifndef _WRITER_
#define _WRITER_
#include "prodmat.h"

// création du fichier de données
// si le fichier existe, il est vidé
// retourne :
//	* le descripteur de fichier
//	* -1 en cas d'échec
int fileCreate(char * fileName);

// lecture de l'entrée utilisateur
// retourne :
// 	* un pointeur vers la structure s_mat
// 	* NULL en cas d'échec
struct s_mat * dataRead(void);

// écriture des données dans le fichier
void dataWrite(int fd, struct s_mat * matStruct);

// projection mémoire
// retourne :
// 	* un pointeur vers la zone de projection
// 	* NULL en cas d'échec
char * fileMap(int fd, char * fileName);

#endif
