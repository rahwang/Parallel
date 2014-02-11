#include "locks.h"

int counter;


/* Worker thread function: increments a counter */
void *increment(void *arg) 
{
  int n = *(int *)arg;

  while(counter != n) {
    counter++;
  }

  pthread_exit(NULL);
}



/* Launches a single thread which increments the counter with
   wild abandon, free to do so because it is running solo */
int serial_work(int work) 
{
  int rc;
  pthread_t *worker = (pthread_t *)malloc(sizeof(pthread_t));
  StopWatch_t watch;

  // Start timing
  startTimer(&watch);

  counter = 0;

  // spawn worker
  if ((rc = pthread_create(worker, NULL, increment, &work))) {
    fprintf(stderr, "error: pthread_create, rc: %d\n", rc);
    exit(1);
  }
  
  // Kill worker
  pthread_join(*worker, NULL);

  // Stop timing
  stopTimer(&watch);

  // print time
  //printf("%f\n",getElapsedTime(&watch));

  // print counter
  printf("Work assigned: %i\nCounter: %i\n", work, counter);

  return 0;
}
  

int main(int argc, char *argv[]) 
{
  if (argc != 2) {
    fprintf(stderr, "Error: wrong number of arguments provided. Program expects 1 work argument.");
    exit(1);
  }

  int work = atoi(argv[1]);

  serial_work(work);

  return 0;
}
