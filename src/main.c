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
 * 
 */
void initPendingMult(Product * prod) {
	size_t i;

	for (i = 0; i < prod->size; i++) {
		prod->pendingMult[i] = 1;
	}
}

/**
 * 
 */
int nbPendingMult(Product * prod) {
	size_t i;
	int nb = 0;
	for (i = 0; i < prod->size; i++) {
		nb += prod->pendingMult[i];
	}

	return(nb);
}

/**
 * 
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

/**
 * Fonction de calcul passée aux threads de multiplication.
 */
void * mult(void * data)
{
size_t index;
size_t iter;

/*=>Recuperation de l'index, c'est a dire index = ... */
index = * ((size_t *) data);

fprintf(stderr,"Begin mult(%d)\n", (int) index);
                                           /* Tant que toutes les iterations */
for(iter=0;iter<prod.nbIterations;iter++)  /* n'ont pas eu lieu              */
 {
 /*=>Attendre l'autorisation de multiplication POUR UNE NOUVELLE ITERATION...*/
 pthread_mutex_lock(&prod.mutex);
 while((prod.state != STATE_MULT) || (prod.pendingMult[index] == 0)) {
  pthread_cond_wait(&prod.cond, &prod.mutex);
 }
 pthread_mutex_unlock(&prod.mutex);

 fprintf(stderr,"--> mult(%d)\n", (int) index); /* La multiplication peut commencer */

 /*=>Effectuer la multiplication a l'index du thread courant... */
 prod.v3[index] = prod.v1[index] * prod.v2[index];

 wasteTime(200+(rand()%200)); /* Perte du temps avec wasteTime() */

 fprintf(stderr,"<-- mult(%d) : %.3g*%.3g=%.3g\n",           /* Affichage du */
         (int) index,prod.v1[(int) index],prod.v2[(int) index],prod.v3[(int) index]);/* calcul sur   */
                                                             /* l'index      */
 /*=>Marquer la fin de la multiplication en cours... */
 pthread_mutex_lock(&prod.mutex);
 prod.pendingMult[index] = 0;

 /*=>Si c'est la derniere... */
 if (nbPendingMult(&prod) == 0)
   {
    /*=>Autoriser le demarrage de l'addition... */
 	  prod.state = STATE_ADD;
   }

 pthread_cond_broadcast(&prod.cond);
 /* libération du mutex */
 pthread_mutex_unlock(&prod.mutex);
 }

fprintf(stderr,"Quit mult(%d)\n", (int) index);
return(data);
}

/*****************************************************************************/
void * add(void * data)
{
size_t iter;
fprintf(stderr,"Begin add()\n");
                                           /* Tant que toutes les iterations */
for(iter=0;iter<prod.nbIterations;iter++)  /* n'ont pas eu lieu              */
  {
  size_t index;

  /*=>Attendre l'autorisation d'addition... */
  pthread_mutex_lock(&prod.mutex);
  while(prod.state != STATE_ADD) {
   pthread_cond_wait(&prod.cond, &prod.mutex);
  }
  pthread_mutex_unlock(&prod.mutex);

  fprintf(stderr,"--> add\n"); /* L'addition peut commencer */

  /* Effectuer l'addition... */
  prod.result=0.0;
  for(index=0;index<prod.size;index++)
  {
   prod.result += prod.v3[index];
  }

  wasteTime(100+(rand()%100)); /* Perdre du temps avec wasteTime() */

  fprintf(stderr,"<-- add\n");

  /*=>Autoriser le demarrage de l'affichage... */
  pthread_mutex_lock(&prod.mutex);
  prod.state = STATE_PRINT;
  pthread_cond_broadcast(&prod.cond);
  
  /* libération du mutex */
  pthread_mutex_unlock(&prod.mutex);
  }

fprintf(stderr,"Quit add()\n");
return(data);
}

int main(int argc, char * argv[]) {
	// depuis l'entrée standard
	s_mat * matStruct;
	int fd;
	int test;

	// lecture entrée utilisateur
	// écriture dans un fichier
	matStruct = dataRead(NULL);
	fd = fileCreate("coucou.txt");
	dataWrite(fd, matStruct);
	close(fd);
	free(matStruct);

	// depuis un fichier
	char * start;
	s_mat * newStruct;

	// projection mémoire du fichier
	start = fileMap("coucou.txt");
	newStruct = dataRead(start);
	free(newStruct);

	/**
	 * 
	 */

	size_t i, iter;
	pthread_t *multTh;
	size_t    *multData;
	pthread_t  addTh;
	void      *threadReturnValue;

	/************* CPUs **************/
	int nbcpus = sysconf(_SC_NPROCESSORS_ONLN); /* utiliser nbcpus-1 pour le modulo */

	/* A cause de warnings lorsque le code n'est pas encore la...*/
	(void)addTh; (void)threadReturnValue;

	/* Lire le fichier de données */

		char * fileName;
		//fileName = argv[1];

		if ((argc <= 1) ||
					(sscanf(argv[1], "%s", fileName) != 1)) {
			fprintf(stderr, "Usage: %s fileName\n", argv[0]);
			return EXIT_FAILURE;
		}

	/* Lire le nombre d'iterations et la taille des vecteurs */

		if ((fd = open(argv[1], O_RDWR)) == -1) {
			perror("open");
			return EXIT_FAILURE;
		}

		// stat taille fichier
		struct stat fileStat;
		stat(fileName, &fileStat);

		// projection mémoire
		char * file = mmap(NULL, (size_t) fileStat.st_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);	

		// fermeture descripteur fichier
		close(fd);

		// lecture données
		char * next;

		prod.nbIterations = (size_t) strtol(file, &next, 10);
		prod.size = (size_t) strtol(next, &next, 10);

	/* Initialisations (Product, tableaux, generateur aleatoire,etc) */

	prod.state=STATE_WAIT;

	prod.pendingMult=(int *)malloc(prod.size*sizeof(int));

	initPendingMult(&prod); /* en vue de la première itération */

	/*=>initialiser prod.mutex ... */
	if (pthread_mutex_init(&prod.mutex, NULL) == -1) {
	perror("pthread_mutex_init");
	return EXIT_FAILURE;
	}

	/*=>initialiser prod.cond ...  */
	if (pthread_cond_init(&prod.cond, NULL) == -1) {
	perror("pthread_cond_init");
	return EXIT_FAILURE;
	}

	/* Allocation dynamique des 3 vecteurs v1, v2, v3 */

	prod.v1=(double *)malloc(prod.size*sizeof(double));
	prod.v2=(double *)malloc(prod.size*sizeof(double));
	prod.v3=(double *)malloc(prod.size*sizeof(double));

	/* Allocation dynamique du tableau pour les threads multiplieurs */

	multTh=(pthread_t *)malloc(prod.size*sizeof(pthread_t));

	/* Allocation dynamique du tableau des MulData */

	multData=(size_t *)malloc(prod.size*sizeof(size_t));

	/* Initialisation du tableau des MulData */

	for(i=0;i<prod.size;i++)
	{
	multData[i]=i;
	}

	/* Threads & CPUs */
	cpu_set_t threads_cpus[nbcpus];
	pthread_attr_t threads_attr[nbcpus];

	for (i = 0; i < prod.size; i++) {
			printf("prod size : %d\n", (int) prod.size);
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

	/*=>Creer les threads de multiplication... */
	for (i = 0; i < prod.size; i++) {
	if (pthread_create(&multTh[i], &threads_attr[i], mult, &multData[i]) == -1) {
	perror("pthread_create");
	return EXIT_FAILURE;
	}
	}

	/*=>Creer le thread d'addition...          */
	if (pthread_create(&addTh, NULL, add, NULL) == -1) {
	perror("pthread_create");
	return EXIT_FAILURE;
	}

	srand(time((time_t *)0));   /* Init du generateur de nombres aleatoires */

	/* Pour chacune des iterations a realiser, c'est a dire :                   */
	for(iter=0;iter<prod.nbIterations;iter++) /* tant que toutes les iterations */
	{                                       /* n'ont pas eu lieu              */
	size_t j;

	/* Initialiser aleatoirement les deux vecteurs */

	for(j=0;j<prod.size;j++)
		{
		prod.v1[j] = strtol(next, &next, 10);
		}

	for(j=0;j<prod.size;j++)
		{
		prod.v2[j] = strtol(next, &next, 10);
		}

	/*=>Autoriser le demarrage des multiplications pour une nouvelle iteration..*/
	pthread_mutex_lock(&prod.mutex);
	initPendingMult(&prod); /* on remet les compteurs à 1 pour la prochaine itération */
	prod.state = STATE_MULT;
	pthread_cond_broadcast(&prod.cond);
	pthread_mutex_unlock(&prod.mutex);

	/*=>Attendre l'autorisation d'affichage...*/
	pthread_mutex_lock(&prod.mutex);
	while(prod.state != STATE_PRINT)
	pthread_cond_wait(&prod.cond, &prod.mutex);
	pthread_mutex_unlock(&prod.mutex);

	/*=>Afficher le resultat de l'iteration courante...*/
	printf("Résultat pour l'itération #%d : %.3g\n", (int) iter + 1, prod.result);
	}

	/* retour des threads */
	int * status;

	/*=>Attendre la fin des threads de multiplication...*/
	for (i = 0; i < prod.size; i++) {
	if (pthread_join(multTh[i], (void *) &status) != 0) {
	perror("pthread_join");
	return EXIT_FAILURE;
	}
	}

	/*=>Attendre la fin du thread d'addition...*/
	if (pthread_join(addTh, (void *) &status) != 0) {
	perror("pthread_join");
	return EXIT_FAILURE;
	}

	/* Libération de status */
	free(status);

	/*=> detruire prod.cond ... */
	pthread_cond_destroy(&prod.cond);

	/*=> detruire prod.mutex ... */
	pthread_mutex_destroy(&prod.mutex);

	/* Detruire avec free ce qui a ete initialise avec malloc */

	free(prod.pendingMult);
	free(prod.v1);
	free(prod.v2);
	free(prod.v3);
	free(multTh);
	free(multData);
	return(EXIT_SUCCESS);
}
