/* Rachel Hwang            */
/* Winter 2014             */
/* Parallel Computing      */
/* Project 1               */

#include <stdlib.h>
#include <stdio.h>
#include "stopwatch.h"
#include <pthread.h>
#include "funcs.h"


int main(int argc, char *argv[])
{
  int n, read;
  StopWatch_t watch;
  
  if (!argv[1])
    printf("Error: no input file name given\n");
  FILE *src = fopen(argv[1], "r");

  // get n value
  read = fscanf(src, "%d\n", &n);
  if (!read)
    printf("Error: no values read\n");
  
  // allocate space for the matrix
  int *a = (int *)malloc(sizeof(int)*n*n);
  
  // Read values into adjacency matrix
  readm(a, n, src);
  
  /*  // Pretty print a matrix for testing.
  printf("\nInput values:\n");
  pprint(a, n); */

  // Start timing
  startTimer(&watch);

  // Floyd-Warshall Algorithm
  floyd(a, n);

  // Stop timing
  stopTimer(&watch);

  /*  // Pretty print a matrix for testing.
  printf("\nOutput values:\n");
  pprint(a, n); */

  // Print elapsed time
  printf("%f\n\n", getElapsedTime(&watch));

  // Output matrix to file
  writem(a, n, "s_output.txt");

  return 0;
}

