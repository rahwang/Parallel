#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "funcs.h"

// Declare mutex 
pthread_mutex_t c_lock;
// Declare barriers
pthread_barrier_t b1, b2;
// Declare counter
int counter = 0;

// Print a matrix
void pprint(int *a, int n) {
  int i, j;
  for (i = 0; i < n; i++) {
    printf("|");
    for (j = 0; j <n; j++) {
      printf("%3i ", a[i*n + j]);
    }
    printf("|\n");
  }
  printf("\n\n");
}


/* Generates a random nxn matrix */
int *randomM(int n)
{
  int i, j;
  int *new = (int *) malloc(n*n*sizeof(int));
  
  srand(time(NULL));
  
  for (i = 0; i < n; i++) {
    for (j = 0; j < n; j++) {
      if (i != j)
	new[i*n+j] = (int) rand() % 20;
      else
	new[i*n+j] = 0;
    }
  } 
  return new;
}


/* Compare two n x n matrices
 * returns number of mismatches */
int compareM(int *a, int *b, int n) {
  int err = 0;
  int i, j;
  for (i = 0; i < n; i++) {
    for (j = 0; j < n; j++) {
      if (a[i*n+j] != b[i*n+j]) {
	err++;
	fprintf(stderr, "Mismatch at [%d, %d]\n", i, j);
      }
    }
  }
  //fprintf(stderr, "There were [ %d ] mismatches.\n", err);
  return err;
}


/* Turns return value into the appropriate string 
 * for display of test results */
char *res(int x) {
  return (x ? "!!! FAIL" : "PASS");
}



// Read in matric values from specified file
void readm(int *a, int n, FILE *src) {
  int i;
  int *cur = a;
  int read = fscanf(src, "%d", &i);
  if (!read)
    printf("Error: no data read\n");
  while (!feof(src)) {
    *cur = i;
    cur++;
    read = fscanf(src, "%d", &i);
  }
  fclose(src);
}


// Output matrix to the specified file
void writem(int *a, int n, char *fname) {
  int i, j;
  FILE *dst = fopen(fname, "w");
  if (!dst){
    fprintf(stderr, "Unable to open file\n");
    exit(1);
  }    
  
  // fprintf(dst, "Updated adjacency matrix (n = %d)\n", n);
  for (i = 0; i < n; i++) {
    for (j = 0; j <n; j++) {
      fprintf(dst, "%i ", a[i*n + j]);
    }
    fprintf(dst, "\n");
  }
  fclose(dst);
}

void floyd(int *a, int n) {
  int i, j, k;
  // Floyd-Warshall Algorithm
  for (k = 0; k < n; k++) {
    for (i = 0; i < n; i++) {
      if (i != k) {
	for (j = 0; j < n; j++) {
	  if (j != k) {
	    if ((a[i*n+k] + a[k*n+j]) < a[i*n+j])
	      a[i*n + j] = a[i*n+k] + a[k*n+j];
	  }
	}
      }
    }
  } 
}


// Make n threads, calling the thr_row func from each
int thread(int num, int n_val, int *a, pthread_t *thr, thr_data_t *thr_data) {
  int i, rc;
  
  // Create threads
  for (i = 0; i < num; i++) {
    thr_data[i].tid = i;
    thr_data[i].n = n_val;
    thr_data[i].matrix = a;
    if ((rc = pthread_create(&thr[i], NULL, thr_row, &thr_data[i]))) {
      fprintf(stderr, "error: pthread_create, rc: %d\n", rc);
      return EXIT_FAILURE;
    }
  }
  return 0;
}

// Thread function: complete a row of the Floyd-Warshall algorithm
void *thr_row(void *arg) {
  thr_data_t *data = (thr_data_t *)arg;
  int *a = data->matrix;
  int n = data->n;
  int i, j, k;
  i = j = k = 0;

  //printf("THREAD ID %d\n", data->tid);
  while (1) {
    // get next row from counter
    pthread_mutex_lock(&c_lock);
    i = counter;
    counter++;
    pthread_mutex_unlock(&c_lock);

    // printf("%d   %d\n", data->tid, i);

    if (i < n) {
      if (i != k) {
	for (j = 0; j < n; j++) {
	  if (j != k) {
	    if ((a[i*n+k] + a[k*n+j]) < a[i*n+j])
	      a[i*n + j] = a[i*n+k] + a[k*n+j];
	  }
	}
      }
    } else {
      k++;
      // printf("%d Waiting for k increment\n", data->tid);
      pthread_barrier_wait(&b1);
      pthread_barrier_wait(&b2);
      if (k == n) {
	pthread_exit(NULL);
      }
    } 
  } 
}
