#ifndef _READER_
#define _READER_
#include "prodmat.h"

/**
 * @brief Projection mémoire pour un fichier désigné par son chemin.
 * 
 * @param fileName Le fichier depuis lequel effectuer la projection.
 * @return char* Un pointeur vers le début de la zone mémoire de projection (NULL en cas d'échec).
 */
char * fileMap(char * fileName);

/**
 * @brief Lecture des données des matrices depuis l'entrée standard ou depuis un pointeur vers le début d'une zone de projection mémoire.
 * 
 * @param start Le pointeur vers la zone de projection, ou NULL si l'on veut lire depuis l'entrée standard.
 * @return struct s_mat* Un pointeur vers la structure résultante (NULL en cas d'échec).
 */
struct s_mat * dataRead(char * start);

#endif
