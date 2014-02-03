/* Rachel Hwang            */
/* Winter 2014             */
/* Parallel Computing      */
/* Project 1               */

#include <stdlib.h>
#include <stdio.h>
#include "stopwatch.h"
#include <pthread.h>
#include <time.h>
#include "funcs.h"


int main(int argc, char *argv[])
{
  int i, k, n, read;
  StopWatch_t watch;
  
  if (!argv[2])
    printf("Error: no input file name argument given\n");
  if (!argv[1])
    printf("Error: no thread number argument given\n");

  FILE *src = fopen(argv[2], "r");

  // get n value
  read = fscanf(src, "%d\n", &n);
  if (!read)
    printf("Error: no values read\n");

  // Get number of threads
  int num = atoi(argv[1]);
    
  // allocate space for the matrix
  int *a = (int *)malloc(sizeof(int)*n*n);
  
  // Read values into adjacency matrix
  readm(a, n, src);
  
  // Start timing
  startTimer(&watch);

  // Initialize mutex
  pthread_mutex_init(&c_lock, NULL);
  // Initialize barriers
  pthread_barrier_init(&b1, NULL, num+1);
  pthread_barrier_init(&b2, NULL, num+1);

  // Create threads
  pthread_t *thr = (pthread_t *)malloc(num*sizeof(pthread_t));
  // Create a thread argument array
  thr_data_t *thr_data = (thr_data_t *)malloc(num*sizeof(thr_data_t));
  // Spawn threads
  thread(num, n, a, thr, thr_data);

  // Floyd-Warshall Algorithm
  k = 0;
  while(k < n) {
    pthread_barrier_wait(&b1);
    k++;
    //pthread_mutex_lock(&c_lock);
    counter = 0;
    //pthread_mutex_unlock(&c_lock);
    pthread_barrier_wait(&b2);
  }

  // Join threads
  for (i = 0; i < num; i++) {
    pthread_join(thr[i], NULL);
  }
  // Destroy mutex
  pthread_mutex_destroy(&c_lock);

  // Stop timing
  stopTimer(&watch);

  /*  // Pretty print a matrix for testing.
  printf("\nOutput values:\n");
  pprint(a, n); */

  // Print elapsed time
  printf("%f\n\n", getElapsedTime(&watch));

  // Output matrix to file
  writem(a, n, "p_output.txt");

  return 0;
}

