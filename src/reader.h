#ifndef _READER_
#define _READER_
#include "prodmat.h"

// lecture des données des matrices depuis un pointeur
// retourne :
// 	* un pointeur vers la structure s_mat résultante
// 	* NULL en cas d'échec
struct s_mat * dataReadFromMem(char * start);

// projection mémoire
// retourne :
// 	* un pointeur vers la zone de projection
// 	* NULL en cas d'échec
char * fileMap(char * fileName);

#endif
