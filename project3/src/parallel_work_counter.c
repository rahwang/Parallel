#include "locks.h"
#include "work_counter.h"

#define TAS 1
#define BACK 2
#define MUTEX 3
#define ALOCK 4
#define CLH 5



/* TSA Worker thread function  */
void *tas(void *arg) {

  thr_data_t *data = (thr_data_t *)arg;
  volatile int *counter = data->counter;

  while(data->my_count) {
    tas_lock(data->state);
    (*counter)++;
    tas_unlock(data->state);
    (data->my_count)--;
  }

  pthread_exit(NULL);
}


/* BACKOFF Worker thread function  */
void *back(void *arg) {

  thr_data_t *data = (thr_data_t *)arg;
  volatile int *counter = data->counter;
  
  while(data->my_count) {
    backoff_lock(data->state, data->backoff);
    (*counter)++;
    backoff_unlock(data->state);
    (data->my_count)--;
  }

  pthread_exit(NULL);
}


/* MUTEX Worker thread function  */
void *mutex(void *arg) 
{
  thr_data_t *data = (thr_data_t *)arg;
  volatile int *counter = data->counter;

  
  while(data->my_count) {
    mutex_lock(data->mutex);
    (*counter)++;
    mutex_unlock(data->mutex);
    (data->my_count)--;
  }

  pthread_exit(NULL);
}


/* ANDERSON Worker thread function  */
void *anders(void *arg) 
{
  thr_data_t *data = (thr_data_t *)arg;
  volatile int *counter = data->counter;
  volatile int idx = 0;

  while(data->my_count) {
    anders_lock(data->alock, &idx);
    (*counter)++;
    anders_unlock(data->alock, &idx);
    (data->my_count)--;
  }

  pthread_exit(NULL);
}


/* CLH Worker thread function  */
void *clh(void *arg) 
{
  thr_data_t *data = (thr_data_t *)arg;
  volatile int *counter = data->counter;

  volatile clh_t lock;
  lock.me = new_clh_node();
  lock.tail = data->clh_tail;

  while(data->my_count) {
    clh_lock(&lock);
    (*counter)++;
    clh_unlock(&lock);
    (data->my_count)--;
  }

  pthread_exit(NULL);
}


/* Spawn workers using the appropriate lock type */
pthread_t *spawn_work(int type, int n,
		      int work, 
		      volatile int *counter, 
		      volatile int *state, 
		      volatile int *backoff, 
		      pthread_mutex_t *m, 
		      volatile alock_t *alock, 
		      volatile node_t **clh_tail, 
		      thr_data_t *data) {

  int i, rc;

  pthread_t *workers = (pthread_t *)malloc(sizeof(pthread_t)*n);

  for (i = 0; i < n; i++) {
    data[i].counter = counter;
    data[i].state = state;
    data[i].backoff = backoff;
    data[i].mutex = m;
    data[i].alock = alock;
    data[i].clh_tail = clh_tail;
    data[i].my_count = work/n;
  }
  
  // spawn workers using correct lock type
  switch (type) {
    
  case TAS:
    for (i = 0; i < n; i++) {
      if ((rc = pthread_create(workers+i, NULL, tas, data+i))) {
	fprintf(stderr, "error: pthread_create, rc: %d\n", rc);
	exit(1);
      }
    }
    break;
  case BACK:
    for (i = 0; i < n; i++) {
      if ((rc = pthread_create(workers+i, NULL, back, data+i))) {
	fprintf(stderr, "error: pthread_create, rc: %d\n", rc);
	exit(1);
      }
    }
    break;
  case MUTEX:
    for (i = 0; i < n; i++) {
      if ((rc = pthread_create(workers+i, NULL, mutex, data+i))) {
	fprintf(stderr, "error: pthread_create, rc: %d\n", rc);
	exit(1);
      }
    }
    break;
  case ALOCK:
    for (i = 0; i < n; i++) {
      if ((rc = pthread_create(workers+i, NULL, anders, data+i))) {
	fprintf(stderr, "error: pthread_create, rc: %d\n", rc);
	exit(1);
      }
    }
    break;
  case CLH:
    for (i = 0; i < n; i++) {
      if ((rc = pthread_create(workers+i, NULL, clh, data+i))) {
	fprintf(stderr, "error: pthread_create, rc: %d\n", rc);
	exit(1);
      }
    }    
  }
  return workers;
}



/* Launches n worker threads increamenting under 
   the authority of given lock */
int parallel_work(int work, int n, int type)
{
  int i;
  StopWatch_t watch;
    
  // Intialize lock args
  volatile int counter = 0;
  volatile int state = 0;
  volatile int backoff = 0;
  pthread_mutex_t m;
  pthread_mutex_init(&m, NULL);
  
  // Initialize alock
  volatile int *anders = (int *)malloc(sizeof(int)*n*4);
  anders[0] = 1;
  for (i = 1; i < n; i++) {
    anders[i*4] = 0;
  }
  volatile int tail = 0;
  volatile alock_t alock;
  alock.array = anders;
  alock.tail = &tail;
  alock.max = n*4;

  // Initialize CLH tail
  volatile node_t *clh_tail = new_clh_node();
  
  thr_data_t *data = (thr_data_t *)malloc(sizeof(thr_data_t)*n);

  // Start timing
  startTimer(&watch);
  
  counter = 0;
  
  // spawn worker
  pthread_t *workers = spawn_work(type, n, work, &counter, &state, &backoff, &m, &alock, &clh_tail, data);

  // Kill worker
  for (i = 0; i < n; i++) {
    pthread_join(workers[i], NULL);
  }
  
  // Stop timing
  stopTimer(&watch);
  
  // print counter
  printf("Counter = %i\n", counter);

  // print thread counters
  for (i = 0; i < n; i++) {
    printf("%i : %i \n", i, data[i].my_count); 
  }

  // Cleanup
  free(workers);
  free(data);

  // print time
  //printf("%f\n",getElapsedTime(&watch));
  
  return 0;
}

int main(int argc, char *argv[])
{
  // get args
  if (argc != 4) 
    {
      fprintf(stderr, "Error: wrong number of arguments provided. Program expects 3 arguments.");
      exit(1);
    }
  
  int work = atoi(argv[1]);
  int n = atoi(argv[2]);
  int type = atoi(argv[3]);

  if (work % n != 0) {
    fprintf(stderr, "Error: work is not divisible by number of worker threads.");
    exit(1);
  }  

  parallel_work(work, n, type);
  
  return 0;
}
