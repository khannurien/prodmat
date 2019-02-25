#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <sched.h>		/* cpu_set_t */
#include <pthread.h>	/* pthread, mutex, cond */
#include <fcntl.h> 		/* open */
#include <unistd.h> 	/* write, sysconf */
#include <signal.h> 	/* sigaction */
#include <sys/mman.h> 	/* mmap, mprotect */
#include <sys/stat.h> 	/* stat */
#include <sys/types.h> 	/* mprotect */
#include <sys/time.h>	/* gettimeofday */
#include "writer.h"
#include "reader.h"
#include "prodmat.h"
#include "product.h"

/**
 * Structure de données pour la multiplication de matrices.
 */
Product prod;

/**
 * Fonction de calcul passée aux threads de multiplication.
 */
void * calc(void * data) {
	size_t index;
	size_t iter;

	// récupération de l'index
	index = * ((size_t *) data);
	fprintf(stderr,"Begin mult(%d)\n", (int) index);

	// tant que toutes les itérations n'ont pas eu lieu...
	for (iter = 0; iter < prod.matrix->nbMult; iter++) {
		// attente de l'autorisation de multiplication pour une nouvelle itération
		pthread_mutex_lock(&prod.mutex);
		while((prod.state != STATE_CALC) || (prod.pendingMult[index] == 0)) {
			pthread_cond_wait(&prod.cond, &prod.mutex);
		}
 		pthread_mutex_unlock(&prod.mutex);

		// on débute la multiplication
		fprintf(stderr,"--> calc(%d)\n", (int) index);

		// calcul des index dans les matrices en fonction de l'index du thread
		int i, j;
		i = (int) index / prod.matrix->matSize[(int) index + 1][1];
		j = (int) index - i * prod.matrix->matSize[(int) index + 1][1];

		// calcul du coefficient à placer dans la matrice résultat
		int coeff = 0;

		int k;
		for (k = 0; k < prod.matrix->matSize[(int) index][0]; k++) {
			coeff += prod.matrix->matTab[(int) index][i][k] * prod.matrix->matTab[(int) index + 1][k][j];
		}

		// affectation à la matrice de résultat
		prod.res[i][j] = coeff;
		printf("%d\n", coeff);

		// perte de temps
		wasteTime(200+(rand()%200));

		// marquer la fin de la multiplication en cours
		pthread_mutex_lock(&prod.mutex);
		prod.pendingMult[index] = 0;

		// si c'est la dernière...
		if (nbPendingMult(&prod) == 0) {
			// on autorise le démarrage de l'écriture
			prod.state = STATE_WRITE;
		}

		// libération du mutex
		pthread_cond_broadcast(&prod.cond);
		pthread_mutex_unlock(&prod.mutex);
	}

	// fin du calcul
	fprintf(stderr,"Quit calc(%d)\n", (int) index);

	return(data);
}

/**
 *
 */
int main(int argc, char * argv[]) {
	size_t i, iter;
	pthread_t *multTh;
	size_t    *multData;
	pthread_t  addTh;
	void      *threadReturnValue;

	/************* CPUs **************/
	int nbcpus = sysconf(_SC_NPROCESSORS_ONLN); /* utiliser nbcpus-1 pour le modulo */

	/* A cause de warnings lorsque le code n'est pas encore la...*/
	(void)addTh; (void)threadReturnValue;

	// récupération du chemin du fichier de données
	char * fileName;
	if ((argc <= 1) || (sscanf(argv[1], "%s", fileName) != 1)) {
		fprintf(stderr, "Usage: %s fileName\n", argv[0]);
		return EXIT_FAILURE;
	}

	// projection mémoire
	char * memData;
	memData = fileMap(fileName);

	// lecture données
	char * start;
	s_mat * matData;
	matData = dataRead(start);

	// initialisation
	prod.state = STATE_WAIT;

	// recherche du nombre maximal de threads, de lignes, de colonnes
	int maxTh = 0;
	int maxL = 0;
	int maxC = 0;
	int n;
	for (n = 0; n < prod.matrix->nbMult; n += 2) {
		// max threads
		int tmpMax = prod.matrix->matSize[n][0] * prod.matrix->matSize[n + 1][1];
		if (tmpMax > maxTh) maxTh = tmpMax;

		// max lignes
		if (prod.matrix->matSize[n][0] > maxL) maxL = prod.matrix->matSize[n][0];

		// max colonnes
		if (prod.matrix->matSize[n + 1][1] > maxC) maxC = prod.matrix->matSize[n + 1][1];
	}

	// affectation
	prod.maxThreads = maxTh;

	// allocation de la matrice de résultat
	if ((prod.res = (int **) malloc(maxL * sizeof(int *))) == NULL) {
		perror("malloc");
		return EXIT_FAILURE;
	}

	int ligne;
	for (ligne = 0; ligne < maxL; ligne++) {
		if ((prod.res[ligne] = (int *) malloc(maxC * sizeof(int))) == NULL) {
			perror("malloc");
			return EXIT_FAILURE;
		}
	}

	// init pendingMult
	prod.pendingMult = (int *) malloc(prod.maxThreads * sizeof(int));
	// préparation de la première itération
	initPendingMult(&prod);

	// init prod.mutex
	if (pthread_mutex_init(&prod.mutex, NULL) == -1) {
		perror("pthread_mutex_init");
		return EXIT_FAILURE;
	}

	// init prod.cond
	if (pthread_cond_init(&prod.cond, NULL) == -1) {
		perror("pthread_cond_init");
		return EXIT_FAILURE;
	}

	// allocation du tableau des threads de calcul
	multTh = (pthread_t *) malloc(prod.maxThreads * sizeof(pthread_t));

	// allocation du tableau des multData
	multData = (size_t *) malloc(prod.maxThreads * sizeof(size_t));

	// init multData
	for (i = 0; i < prod.maxThreads; i++) {
		multData[i] = i;
	}

	/* Threads & CPUs */
	cpu_set_t threads_cpus[nbcpus];
	pthread_attr_t threads_attr[nbcpus];

	for (i = 0; i < prod.maxThreads; i++) {
		/* Initialisation des struct attr */
		pthread_attr_init(&threads_attr[i]);
		/* RAZ */
		CPU_ZERO(&threads_cpus[i]);
		/* Attribution */
			printf("i : %d\n", (int) i);
			printf("nbcpus : %d\n", nbcpus);
		CPU_SET(i % nbcpus, &threads_cpus[i]);
		/*
		*        int pthread_attr_setaffinity_np(pthread_attr_t *attr,
		*                                  size_t cpusetsize, const cpu_set_t *cpuset);
		*/
		pthread_attr_setaffinity_np(&threads_attr[i], sizeof(threads_cpus[i]), &threads_cpus[i]);
	}

	// création des threads de multiplication
	for (i = 0; i < prod.maxThreads; i++) {
		if (pthread_create(&multTh[i], &threads_attr[i], calc, &multData[i]) == -1) {
			perror("pthread_create");
		return EXIT_FAILURE;
		}
	}

	// tant que toutes les itérations n'ont pas eu lieu...
	for (iter = 0; iter < prod.matrix->nbMult; iter++) {
		size_t j;

		// calcul du nombre de threads effectifs
		prod.nbThreads = prod.matrix->matSize[n][0] * prod.matrix->matSize[n + 1][1];

		pthread_mutex_lock(&prod.mutex);
		// on remet les compteurs à 1 pour la prochaine itération
		initPendingMult(&prod);
		// autorisation de démarrage des multiplications pour une nouvelle itération
		prod.state = STATE_CALC;
		pthread_cond_broadcast(&prod.cond);
		pthread_mutex_unlock(&prod.mutex);

		// attente de l'autorisation d'écriture
		pthread_mutex_lock(&prod.mutex);
		while(prod.state != STATE_WRITE) {
			pthread_cond_wait(&prod.cond, &prod.mutex);
		}
		pthread_mutex_unlock(&prod.mutex);

		// écriture de la matrice résultat dans le fichier
		// TODO: ...
	}

	// attente de la fin des multiplications
	int * status;
	for (i = 0; i < prod.maxThreads; i++) {
		if (pthread_join(multTh[i], (void *) status) != 0) {
			perror("pthread_join");
			return EXIT_FAILURE;
		}
	}

	// libération de status
	free(status);

	// destruction de cond
	pthread_cond_destroy(&prod.cond);

	// destruction du mutex
	pthread_mutex_destroy(&prod.mutex);

	// libérations
	free(prod.pendingMult);
	free(multTh);
	free(multData);
	return(EXIT_SUCCESS);
}
