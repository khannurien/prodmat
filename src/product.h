#ifndef _PRODUCT_
#define _PRODUCT_
#define _GNU_SOURCE
#include <sys/types.h>	/* pthread, size_t */

/**
 * 
 */
typedef enum {
	STATE_WAIT,
	STATE_MULT,
	STATE_ADD,
  	STATE_PRINT
} State;

/**
 * 
 */
typedef struct {
	State state;
	int * pendingMult;
	pthread_cond_t cond;
	pthread_mutex_t mutex;
	size_t nbIterations;
	size_t size;
	double * v1;
	double * v2;
	double * v3;
	double result;
} Product;

/**
 * 
 */
void * mult(void * data);

/**
 * 
 */
void * add(void * data);

#endif
