#ifndef _PRODUCT_
#define _PRODUCT_
#define _GNU_SOURCE
#include <sys/types.h>	/* pthread, size_t */
#include "prodmat.h"

/**
 * @brief L'état courant du programme.
 * 
 */
typedef enum {
	STATE_WAIT,	// programme en attente
	STATE_CALC, // autorisation de début d'une multiplication
  	STATE_WRITE // autorisation d'écriture du fichier résultat
} State;

/**
 * @brief La structure de données du programme de calcul. 
 * 
 */
typedef struct {
	State state; // l'état courant du programme
	int * pendingMult; // le tableau des multiplications à réaliser
	pthread_cond_t cond; // la variable condition
	pthread_mutex_t mutex; // le sémaphore d'exclusion mutuelle
	size_t maxThreads; // le nombre maximum de threads requis pour un produit matriciel
	size_t nbThreads; // le nombre de threads effectivement engagés dans un produit matriciel
	s_mat * matrix; // un pointeur vers la structure contenant les matrices (tailles et valeurs)
	int ** res; // matrice de résultat, pour une itération
} Product;

/**
 * @brief Initialiser le tableau pendingMult. Les valeurs sont initialisées à 2
 * pour tous les threads, et lors d'une itération, les valeurs dans le tableau
 * pour les threads utiles sont passées à 1.
 * 
 * @param prod La structure Product du programme de calcul.
 */
void initPendingMult(Product * prod);

/**
 * @brief Trouver le nombre de multiplications en attente.
 * 
 * @param prod La structure Product du programme de calcul.
 * @return int Le nombre de multiplications restantes.
 */
int nbPendingMult(Product * prod);

#endif
