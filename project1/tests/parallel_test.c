#include <stdio.h>
#include <string.h>
#include "../src/funcs.h"


// Counter for thread() testing + lock
int t;
pthread_mutex_t t_lock;


/* Helper for testThread - just a sample function */
void *go(void *arg) {
  thr_data_t *data = (thr_data_t *)arg;
  if (data->n) {
    pthread_mutex_lock(&t_lock);
    t++;
    pthread_mutex_unlock(&t_lock);
  }
  pthread_exit(NULL);
}


/* Test of thread()
 * checks that all threads spawn as expected
 * (uses modified thread() function - calls test function "go()")
 * instead of thr_row()
 */
int testTHREAD(int num) {
  int i, rc;
  int n = 10;
  int *a = &n;

  // Initialize mutex
  pthread_mutex_init(&t_lock, NULL);

  // Create array of threads
  pthread_t *thr = (pthread_t *)malloc(num*sizeof(pthread_t));
  // Create a thread argument array
  thr_data_t *thr_data = (thr_data_t *)malloc(num*sizeof(thr_data_t));

  // Reset counter
  t = 0;

  // Create threads
  for (i = 0; i < num; i++) {
    thr_data[i].tid = i;
    thr_data[i].n = n;
    thr_data[i].matrix = a;
    if ((rc = pthread_create(&thr[i], NULL, go, &thr_data[i]))) {
      fprintf(stderr, "error: pthread_create, rc: %d\n", rc);
      return EXIT_FAILURE;
    }
  }

  // Join threads
  for (i = 0; i < num; i++) {
    pthread_join(thr[i], NULL);
  }

  // All threads check in?
  if (t < num)
    return 1;

  return 0;
}


/* Does the work of floyd_parallel.c */
void f_p(int *a, int n, int num)
{
  int i, k;
  
  counter = 0;

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
}


/* Check behavior of the thr_row() code */
int testROW(int n) {
  int i, j, k = 0;
  int *a = randomM(n);
  int *b = (int *)malloc(n*n*sizeof(int));

  counter = 0;
  
  memcpy(b, a, n*n*sizeof(int));

  while (k < n) {
    pthread_mutex_lock(&c_lock);
    i = counter;
    counter++;
    pthread_mutex_unlock(&c_lock);

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
      counter = 0;
      k++;
    }
  }
  
  floyd(b, n);

  return compareM(a, b, n);
}


/* Checks for correctness, comparing results of 
 * serial and parallel implementations */
int testFLOYD(int n, int num) {
  int *a = randomM(n);
  int *b = (int *)malloc(n*n*sizeof(int));
  
  memcpy(b, a, n*n*sizeof(int));

  floyd(a, n);
  f_p(b, n, num);
  return compareM(a, b, n);
}  


int main() {
  int i, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12;

  // Since this is testing a threaded program, 
  // run threaded tests 100 times
  t1 = t2 = t3 = t4 = t5 = t6 = t7 = t8 = t9 = t10 = t11 = t12 = 0;
  for (i = 0; i < 100; i++) {
    t1 += testTHREAD(1);
    t10 += testTHREAD(3);
    t2 += testTHREAD(8);
    t5 += testFLOYD(10, 1);
    t11 += testFLOYD(10, 3);
    t6 += testFLOYD(10, 8);
    t7 += testFLOYD(101, 1);
    t12 += testFLOYD(101, 3);
    t8 += testFLOYD(101, 8);
  }
  t3 = testROW(10);
  t4 = testROW(11);
  t9 = testROW(100);
  
  printf("\nTESTS for parallel_floyd.c\n\n");

  printf("| %25s: %20s |\n", "THREAD1 (t = 1)", res(t1));
  printf("| %25s: %20s |\n", "THREAD2 (t = 8)", res(t2));
  printf("| %25s: %20s |\n", "ROW1 (n = 10)", res(t3));
  printf("| %25s: %20s |\n", "ROW2 (n = 11)", res(t4));
  printf("| %25s: %20s |\n", "ROW3 (n = 100)", res(t9));
  printf("| %25s: %20s |\n", "FLOYD1 (n = 10, t = 1)", res(t5));
  printf("| %25s: %20s |\n", "FLOYD2 (n = 10, t = 3)", res(t11));
  printf("| %25s: %20s |\n", "FLOYD3 (n = 10, t = 8)", res(t6));
  printf("| %25s: %20s |\n", "FLOYD4 (n = 101, t = 1)", res(t7));
  printf("| %25s: %20s |\n", "FLOYD2 (n = 101, t = 3)", res(t12)); 
  printf("| %25s: %20s |\n", "FLOYD6 (n = 101, t = 8)", res(t8));

  return 0;
}
