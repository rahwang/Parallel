#ifndef FUNCS_H
#define FUNCS_H

/* Functions used in floyd_serial.c and floyd_parallel.c */

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

// Print a matrix
void pprint(int *a, int n);

// Generates a random nxn matrix for testing (if i == j, then 0)
int *randomM(int n);

// Compare two n x n matrices returns number of mismatches
int compareM(int *a, int *b, int n);

// Turns return value into the appropriate string 
// for display of test results
char *res(int x);

// Read in matric values from specified file
void readm(int *a, int n, FILE *src);

// Output matrix to the specified file
void writem(int *a, int n, char *fname);

// Run Floyd-Warshall Algorithm
void floyd(int *a, int n);


/* Used exclusively in parallel program from this point down */

// Declare mutex 
extern pthread_mutex_t c_lock;
// Declare barriers
extern pthread_barrier_t b1, b2;
// Declare counter
extern int counter;

// Thread argument struct for thread function
typedef struct thr_data_t {
  int tid;
  int *matrix;
  int n;
} thr_data_t;

// Make n threads, calling the thr_row func from each
int thread(int num, int n_val, int *a, pthread_t *thr, thr_data_t *thr_data);

// Thread function: complete a row of the Floyd-Warshall algorithm
void *thr_row(void *arg);

#endif
