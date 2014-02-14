#include "locks.h"
#include "time_counter.h"

volatile long long int counter;
volatile int go;


/* Worker thread function: increments a counter */
void *increment() 
{
  while(go) {
    counter++;
  }

  pthread_exit(NULL);
}



/* Launches a single thread which increments the counter with
   wild abandon, free to do so because it is running solo */
int serial_time(unsigned int time) 
{
  int rc;
  pthread_t *worker = (pthread_t *)malloc(sizeof(pthread_t));
  StopWatch_t watch;
  
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
  usleep(time*1000);

  // Kill worker
  go = 0;
  pthread_join(*worker, NULL);

  // Stop timing
  stopTimer(&watch);


  return counter;
}
  

int main(int argc, char *argv[]) 
{
  if (argc != 2) {
    fprintf(stderr, "Error: wrong number of arguments provided. Program expects 1 time argument.");
    exit(1);
  }

  unsigned int time = (unsigned int)atoi(argv[1]);

  serial_time(time);

  // print counter
  printf("Counter = %lli\n", counter);

  return 0;
}
