#ifndef _PRODMAT_
#define _PRODMAT_

/**
 * @brief Structure de données représentant un ensemble quelconque de matrices
 * de tailles variables.
 * La taille de la matrice 0 se lit telle que son nombre de lignes est stocké
 * dans matSize[0][0] et son nombre de colonnes dans matSize[0][1].
 * Les valeurs pour la matrice 0 sont stockées dans matTab[0][1][2] pour sa
 * ligne 1 en colonne 2.
 * Le nombre de matrices dans la structure est toujours égal à deux fois le
 * nombre de multiplications de matrices prévu dans nbMult.
 */
typedef struct s_mat {
	int nbMult; // nombre de multiplications à réaliser
	int nbMat; // nombre de matrices
	int ** matSize; // tableau des tailles de matrices
	int *** matTab; // tableau des matrices
} s_mat;

#endif
