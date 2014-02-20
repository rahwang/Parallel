#include "time_counter.h"
#include "locks.h"

#define TAS 1
#define BACK 2
#define MUTEX 3
#define ALOCK 4
#define CLH 5

volatile long int scounter;
volatile int go;


/* Worker thread function: increments a counter */
void *s_time_worker() 
{
  while(go) {
    scounter++;
  }

  pthread_exit(NULL);
}



/* Launches a single thread which increments the counter with
   wild abandon, free to do so because it is running solo */
long serial_time(unsigned int time) 
{
  int rc;
  pthread_t *worker = (pthread_t *)malloc(sizeof(pthread_t));
  StopWatch_t watch;
  
  // Start timing
  startTimer(&watch);

  scounter = 0;
  go = 1;

  // spawn worker
  if ((rc = pthread_create(worker, NULL, s_time_worker, NULL))) {
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

  // printf("Counter = %lli\n", scounter);

  return scounter;
}

/* Worker thread function  */
void *p_time_worker(void *arg) {

  thr_data_t *data = (thr_data_t *)arg;
  volatile long *counter = data->counter;
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
    if ((rc = pthread_create(workers+i, NULL, p_time_worker, data+i))) {
      fprintf(stderr, "error: pthread_create, rc: %d\n", rc);
      exit(1);
    }
  }  
}



/* Launches n worker threads increamenting under 
   the authority of given lock */
long parallel_time(unsigned int time, int n, int type)
{
  int i;
  StopWatch_t watch;

  // Lock args
  volatile long int counter = 0;
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
    tail = 0;
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
    p->locked = 0;
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
  //printf("Counter = %i\n", counter);
  // print thread counters
  int sum = 0;
  for (i = 0; i < n; i++) {
    //printf("%i : %i \n", i, data[i].my_count);
    sum += data[i].my_count;
  }

  // print time
  //printf("%f\n",getElapsedTime(&watch));
  
  if (counter - sum) {
    return 0;
  }
  return counter;
}
