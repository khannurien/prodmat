#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>	/* gettimeofday */
#include "prodmat.h"
#include "product.h"

/**
 * 
 */
void initPendingMult(Product * prod) {
	size_t i;
	
	for(i = 0; i < prod->maxThreads; i++) {
		prod->pendingMult[i] = 2;
	}

	for (i = 0; i < prod->nbThreads; i++) {
		prod->pendingMult[i] = 1;
	}
}

/**
 * 
 */
int nbPendingMult(Product * prod) {
	size_t i;
	int nb = 0;
	for (i = 0; i < prod->maxThreads; i++) {
		nb += prod->pendingMult[i];
	}

	return(nb);
}

/**
 *
 * Exemple : wasteTime(200+(rand()%200));
 */
void wasteTime(unsigned long ms) {
	unsigned long t, t0;
	struct timeval tv;
	gettimeofday(&tv, (struct timezone *) 0);
	t0 = tv.tv_sec * 1000LU + tv.tv_usec / 1000LU;

	do {
		gettimeofday(&tv,(struct timezone *)0);
		t = tv.tv_sec * 1000LU + tv.tv_usec / 1000LU;
	} while (t - t0 < ms);
}
