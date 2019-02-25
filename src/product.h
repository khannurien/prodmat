#ifndef _PRODUCT_
#define _PRODUCT_
#define _GNU_SOURCE
#include <sys/types.h>	/* pthread, size_t */
#include "prodmat.h"

/**
 * 
 */
typedef enum {
	STATE_WAIT,
	STATE_CALC,
  	STATE_WRITE
} State;

/**
 * 
 */
typedef struct {
	State state;
	int * pendingMult;
	pthread_cond_t cond;
	pthread_mutex_t mutex;
	size_t maxThreads; // le nombre maximum de threads requis pour un produit matriciel
	size_t nbThreads; // le nombre de threads effectivement engagés dans un produit matriciel
	s_mat * matrix; // un pointeur vers la structure contenant les matrices (tailles et valeurs)
	int ** res; // matrice de résultat
} Product;

/**
 * 
 */
void initPendingMult(Product * prod);

/**
 * 
 */
int nbPendingMult(Product * prod);

/**
 * 
 */
void wasteTime(unsigned long ms);

/**
 * Fonction de calcul passée aux threads.
 */
void * calc(void * data);

#endif
