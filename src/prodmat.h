#ifndef _PRODMAT_
#define _PRODMAT_

typedef struct s_mat {
	int nbMult; // nombre de multiplications à réaliser
	int nbMat; // nombre de matrices
	int ** matSize; // tableau des tailles de matrices
	int *** matTab; // tableau des matrices
} s_mat;

#endif
