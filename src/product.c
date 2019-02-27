#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>	/* gettimeofday */
#include "prodmat.h"
#include "product.h"

/**
 * @brief Initialiser le tableau pendingMult. Les valeurs sont initialisées à 2
 * pour tous les threads, et lors d'une itération, les valeurs dans le tableau
 * pour les threads utiles sont passées à 1.
 * 
 * @param prod La structure Product du programme de calcul.
 */
void initPendingMult(Product * prod) {
	size_t i;
	
	for (i = 0; i < prod->maxThreads; i++) {
		prod->pendingMult[i] = 2;
	}

	for (i = 0; i < prod->nbThreads; i++) {
		prod->pendingMult[i] = 1;
	}
}

/**
 * @brief Trouver le nombre de multiplications en attente.
 * 
 * @param prod La structure Product du programme de calcul.
 * @return int Le nombre de multiplications restantes.
 */
int nbPendingMult(Product * prod) {
	size_t i;
	int nb = 0;
	for (i = 0; i < prod->maxThreads; i++) {
		nb += prod->pendingMult[i];
	}

	return(nb);
}
