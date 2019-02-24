#ifndef _READER_
#define _READER_
#include "prodmat.h"

/**
 * Projection mémoire pour un fichier désigné par son chemin.
 * Retourne un pointeur vers la zone de projection, ou NULL en cas d'échec.
 */
char * fileMap(char * fileName);

/**
 * Lecture des données des matrices depuis un pointeur.
 * Retourne un pointeur vers la structure s_mat résultante, ou NULL en cas d'échec.
 */
struct s_mat * dataRead(char * start);

#endif
