#include "locks.h"

int counter;
volatile int go;


/* Worker thread function: increments a counter */
void *increment() {
  while(go) {
    counter++;
  }

  pthread_exit(NULL);
}



/* Launches a single thread which increments the counter with
   wild abandon, free to do so because it is running solo */
int main(int argc, char *argv[]) {

  int rc;
  pthread_t *worker = (pthread_t *)malloc(sizeof(pthread_t));
  StopWatch_t watch;

  if (argc != 2) {
    fprintf(stderr, "Error: wrong number of arguments provided. Program expects 1 time argument.");
    exit(1);
  }

  const int time = atoi(argv[1]);
  
  // Start timing
  startTimer(&watch);

  counter = 0;
  go = 1;

  // spawn worker
  if ((rc = pthread_create(worker, NULL, increment, NULL))) {
    fprintf(stderr, "error: pthread_create, rc: %d\n", rc);
    exit(1);
  }
  
  // sleep
  nanosleep((struct timespec[]){{0, 1000000*time}}, NULL);

  // Kill worker
  go = 0;
  pthread_join(*worker, NULL);

  // Stop timing
  stopTimer(&watch);

  // print counter
  printf("Counter = %i\n", counter);

  return 0;
}
  
