#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>		/* strcat */
#include <sched.h>		/* cpu_set_t */
#include <pthread.h>	/* pthread, mutex, cond */
#include <fcntl.h> 		/* open */
#include <unistd.h> 	/* write, sysconf */
#include <signal.h> 	/* sigaction */
#include <sys/mman.h> 	/* mmap, mprotect */
#include <sys/stat.h> 	/* stat */
#include <sys/types.h> 	/* mprotect */
#include <errno.h>		/* errno, EINVAL, EBUSY */
#include "writer.h"
#include "reader.h"
#include "prodmat.h"
#include "product.h"

/**
 * @brief Structure de données pour la multiplication de matrices.
 * 
 */
Product prod;

/**
 * @brief Fonction de calcul de produit matriciel passée aux threads.
 * 
 * @param data L'index du thread qui exécute la fonction dans le tableau multData.
 * @return void* L'index du thread qui exécute la fonction dans le tableau multData (NULL en cas d'erreur).
 */
void * calc(void * data) {
	size_t index;
	size_t iter;

	// récupération de l'index
	index = * ((size_t *) data);
	fprintf(stderr, "Begin calc(%d)\n", (int) index);

	// tant que toutes les itérations n'ont pas eu lieu...
	for (iter = 0; iter < prod.matrix->nbMult; iter++) {
		// attente de l'autorisation de multiplication pour une nouvelle itération
		if (pthread_mutex_lock(&prod.mutex) == EINVAL) return NULL;
		while((prod.state != STATE_CALC) || (prod.pendingMult[index] == 0)) {
			pthread_cond_wait(&prod.cond, &prod.mutex);
		}
 		if (pthread_mutex_unlock(&prod.mutex) == EINVAL) return NULL;

		// on débute la multiplication
		fprintf(stderr, "--> calc(%d)\n", (int) index);

		// éviter les calculs inutiles -- et les segfault par la même occasion...
		if (prod.pendingMult[index] != 2) {
			// calcul des index dans les matrices en fonction du numéro d'itération
			int iMat = (((int) iter) * 2);
			int i, j;
			i = index / prod.matrix->matSize[iMat + 1][1];
			j = index - i * prod.matrix->matSize[iMat + 1][1];

			// calcul du coefficient à placer dans la matrice résultat
			int coeff = 0;

			int k;
			for (k = 0; k < prod.matrix->matSize[iMat][1]; k++) {
				coeff += prod.matrix->matTab[iMat][i][k] * prod.matrix->matTab[iMat + 1][k][j];
			}

			// affectation à la matrice de résultat
			if (pthread_mutex_lock(&prod.mutex) == EINVAL) return NULL;
			prod.res[i][j] = coeff;
 			if (pthread_mutex_unlock(&prod.mutex) == EINVAL) return NULL;

		}

		// marquer la fin de la multiplication en cours
		if (pthread_mutex_lock(&prod.mutex) == EINVAL) return NULL;
		prod.pendingMult[index] = 0;

		// si c'est la dernière...
		if (nbPendingMult(&prod) == 0) {
			// on autorise le démarrage de l'écriture
			prod.state = STATE_WRITE;
		}

		// libération du mutex
		pthread_cond_broadcast(&prod.cond);
 		if (pthread_mutex_unlock(&prod.mutex) == EINVAL) return NULL;
	}

	// fin du calcul
	fprintf(stderr, "Quit calc(%d)\n", (int) index);

	return(data);
}

/**
 * @brief Programme de calcul de produits matriciels.
 */
int main(int argc, char * argv[]) {
	size_t i, iter;
	pthread_t *multTh;
	size_t    *multData;
	pthread_t  addTh;
	void      *threadReturnValue;

	// récupération du chemin du fichier de données
	char * fileName = NULL;
	if (argc == 2) {
		fileName = argv[1];
	} else {
		printf("Usage: %s <fileName>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	// projection mémoire
	char * memData;
	if ((memData = fileMap(fileName)) == NULL) {
		perror("fileMap");
		exit(EXIT_FAILURE);
	}

	// lecture données
	prod.matrix = dataRead(memData);

	// initialisation
	prod.state = STATE_WAIT;

	// recherche du nombre maximal de threads, de lignes, de colonnes
	int maxTh = 0;
	int maxL = 0;
	int maxC = 0;
	int n;
	for (n = 0; n < prod.matrix->nbMult; n++) {
		// max lignes
		if (prod.matrix->matSize[n * 2][0] > maxL) maxL = prod.matrix->matSize[n * 2][0];

		// max colonnes
		if (prod.matrix->matSize[n * 2 + 1][1] > maxC) maxC = prod.matrix->matSize[n * 2 + 1][1];
	}

	// calcul nb threads max
	prod.maxThreads = maxL * maxC;

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
	if ((prod.pendingMult = (int *) malloc(prod.maxThreads * sizeof(int))) == NULL) {
		perror("malloc");
		exit(EXIT_FAILURE);
	}
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

	// obtention du nombre de CPUs sur la machine
	int nbcpus;
	if ((nbcpus = sysconf(_SC_NPROCESSORS_ONLN)) == -1) {
		perror("sysconf");
		exit(EXIT_FAILURE);
	}

	// affinité des threads sur les CPUs
	cpu_set_t threads_cpus[nbcpus];
	pthread_attr_t threads_attr[nbcpus];

	for (i = 0; i < prod.maxThreads; i++) {
		int indexMod = i % nbcpus;
		// init des structures d'attributs
		if (pthread_attr_init(&threads_attr[indexMod]) != 0) {
			perror("pthread_attr_init");
			exit(EXIT_FAILURE);
		}
		// RAZ
		CPU_ZERO(&threads_cpus[indexMod]);
		// ajout d'un CPU à l'ensemble
		CPU_SET(indexMod, &threads_cpus[indexMod]);
		// mise en place de l'affinité du thread sur cet ensemble
		if (pthread_attr_setaffinity_np(&threads_attr[indexMod], sizeof(threads_cpus[indexMod]), &threads_cpus[indexMod]) != 0) {
			perror("pthread_att_setaffinity_np");
			exit(EXIT_FAILURE);
		}
	}

	// création des threads de multiplication
	for (i = 0; i < prod.maxThreads; i++) {
		if (pthread_create(&multTh[i], &threads_attr[i % nbcpus], calc, &multData[i]) == -1) {
			perror("pthread_create");
			return EXIT_FAILURE;
		}
	}

	// création du fichier de résultat
	int fdRes;
	char resFile[256];
	strcpy(resFile, fileName);
	strcat(resFile, ".res");
	if ((fdRes = fileCreate(resFile)) == -1) {
		perror("fileCreate");
		exit(EXIT_FAILURE);
	}

	// tant que toutes les itérations n'ont pas eu lieu...
	for (iter = 0; iter < prod.matrix->nbMult; iter++) {
		// prise du mutex
		if (pthread_mutex_lock(&prod.mutex) == EINVAL) return NULL;

		// index des matrices à partir du numéro d'itération
		int iMat = (((int) iter) * 2);

		// calcul du nombre de threads effectifs pour l'itération en cours
		prod.nbThreads = prod.matrix->matSize[iMat][0] * prod.matrix->matSize[iMat + 1][1];

		// on remet les compteurs à 1 pour la prochaine itération
		initPendingMult(&prod);

		// autorisation de démarrage des multiplications pour une nouvelle itération
		prod.state = STATE_CALC;
		pthread_cond_broadcast(&prod.cond);
 		if (pthread_mutex_unlock(&prod.mutex) == EINVAL) return NULL;

		// attente de l'autorisation d'écriture
		if (pthread_mutex_lock(&prod.mutex) == EINVAL) return NULL;
		while (prod.state != STATE_WRITE) {
			pthread_cond_wait(&prod.cond, &prod.mutex);
		}

		// écriture de la matrice résultat dans le fichier
		resWrite(fdRes, prod.res, prod.matrix->matSize[iMat][0], prod.matrix->matSize[iMat + 1][1]);

		// libération du mutex
 		if (pthread_mutex_unlock(&prod.mutex) == EINVAL) return NULL;
	}

	// fermeture du fichier de résultat
	close(fdRes);

	// attente de la fin des multiplications
	for (i = 0; i < prod.maxThreads; i++) {
		if (pthread_join(multTh[i], &threadReturnValue) != 0) {
			perror("pthread_join");
			return EXIT_FAILURE;
		}
	}

	// destruction de cond
	if (pthread_cond_destroy(&prod.cond) == EBUSY) exit(EXIT_FAILURE);

	// destruction du mutex
	if (pthread_mutex_destroy(&prod.mutex) == EBUSY) exit(EXIT_FAILURE);

	// libérations
	free(prod.pendingMult);
	free(multTh);
	free(multData);
	return(EXIT_SUCCESS);
}
