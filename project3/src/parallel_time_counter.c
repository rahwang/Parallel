#include "locks.h"
#include "time_counter.h"

#define TAS 1
#define BACK 2
#define MUTEX 3
#define ALOCK 4
#define CLH 5

volatile int go;


/* Worker thread function  */
void *p_time_work(void *arg) {

  thr_data_t *data = (thr_data_t *)arg;
  volatile int *counter = data->counter;
  volatile lock_t *locks = data->locks;
  void (*lockf)(volatile lock_t *) = data->lock_f;
  void (*unlockf)(volatile lock_t *) = data->unlock_f;

  while(go) {
    (*lockf)(locks);
    (*counter)++;
    (*unlockf)(locks);
    (data->my_count)++;
  }

  pthread_exit(NULL);
}

/* Spawn workers using the appropriate lock type */
void spawn_time(int type,
		int n,
		pthread_t *workers,
		thr_data_t *data) 
{
  int i, rc;
  
  for (i=0; i<n;i++) {
    if ((rc = pthread_create(workers+i, NULL, p_time_work, data+i))) {
      fprintf(stderr, "error: pthread_create, rc: %d\n", rc);
      exit(1);
    }
  }  
}



/* Launches n worker threads increamenting under 
   the authority of given lock */
int parallel_time(unsigned int time, int n, int type)
{
  int i;
  StopWatch_t watch;

  // Lock args
  volatile int counter = 0;
  // TAS args
  volatile int state;
  // MUTEX args
  pthread_mutex_t m;
  // Initialize alock
  volatile int anders[n*4]; 
  volatile int tail;
  volatile int head;
  volatile alock_t alock;
  // Initialize CLH tail
  volatile node_t *p;
  
  thr_data_t data[n];
  pthread_t workers[n];
  lock_t lock;
  // Or for clh
  lock_t c_locks[n];

  // Initialize using switch over type
  switch (type) {

  case TAS:
    state = 0;
    lock.tas = &state;
    for (i = 0; i < n; i++) {
      data[i].lock_f = &tas_lock;
      data[i].unlock_f = &tas_unlock;
      data[i].locks = &lock;
    }
    break;
  case BACK:
    state = 0;
    lock.tas = &state;
    for (i = 0; i < n; i++) {
      data[i].lock_f = &backoff_lock;
      data[i].unlock_f = &backoff_unlock;
      data[i].locks = &lock;
    }
    break;
  case MUTEX:
    pthread_mutex_init(&m, NULL);
    lock.m = &m;
    for (i = 0; i < n; i++) {
      data[i].lock_f = &mutex_lock;
      data[i].unlock_f = &mutex_unlock;
      data[i].locks = &lock;
    }
    break;
  case ALOCK:
    tail = 1;
    alock.tail = &tail;
    alock.head = &head;
    alock.max = n*4;
    alock.array = anders;
    for (i = 0; i < n; i++) {
      anders[i*4] = 0;
      data[i].lock_f = &anders_lock;
      data[i].unlock_f = &anders_unlock;
      data[i].locks = &lock;
    }
    anders[0] = 1;
    lock.a = alock;
    break;
  case CLH:
    p = new_clh_node();
    for (i = 0; i < n; i++) {
      data[i].lock_f = &clh_lock;
      data[i].unlock_f = &clh_unlock;
      data[i].locks = c_locks+i;
      c_locks[i].clh.me = new_clh_node();
      c_locks[i].clh.tail = &p;
    }
  }  

  for (i=0; i<n; i++) {
    data[i].counter = &counter;
    data[i].my_count = 0;
  }
		   
  // Start timing
  startTimer(&watch);
  
  counter = 0;
  go = 1;
  
  // spawn worker
  spawn_time(type, n, workers, data);
  
  // sleep
  usleep(time*1000);
  
  // Kill worker
  go = 0;
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
  
  unsigned int time =(unsigned int)atoi(argv[1]);
  int n = atoi(argv[2]);
  int type = atoi(argv[3]);

  parallel_time(time, n, type);
  
  return 0;
}


