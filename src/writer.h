#ifndef _WRITER_
#define _WRITER_
#include "prodmat.h"

/**
 * @brief Création d'un fichier de données désigné par son chemin.
 * Si le fichier existe, il est vidé.
 * La fonction retourne le descripteur de fichier, ou -1 en cas d'échec.
 * 
 * @param fileName Le chemin du fichier à créer.
 * @return int Le descripteur de fichier associé.
 */
int fileCreate(char * fileName);

/**
 * @brief Écriture des données dans un fichier désigné par son descripteur.
 * 
 * @param fd Le descripteur du fichier dans lequel écrire.
 * @param matStruct La structure contenant les données de la matrice
 */
void dataWrite(int fd, struct s_mat * matStruct);

/**
 * @brief Écriture du résultat d'un produit matriciel dans un fichier.
 * 
 * @param fd Le descripteur du fichier dans lequel écrire.
 * @param matrix La matrice de résultat à écrire.
 * @param maxL Le nombre de lignes dans la matrice résultat.
 * @param maxC Le nombre de colonnes dans la matrice résultat.
 */
void resWrite(int fd, int ** matrix, int maxL, int maxC);

#endif
