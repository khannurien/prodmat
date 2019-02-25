#ifndef _WRITER_
#define _WRITER_
#include "prodmat.h"

/**
 * Création d'un fichier de données désigné par son chemin.
 * Si le fichier existe, il est vidé.
 * La fonction retourne le descripteur de fichier, ou -1 en cas d'échec.
 */
int fileCreate(char * fileName);

/**
 * Écriture des données dans un fichier désigné par son descripteur.
 * Prend une structure s_mat en entrée.
 */
void dataWrite(int fd, struct s_mat * matStruct);

/**
 * 
 */
void resWrite(int fd, int ** matrix, int maxL, int maxC);

#endif
