#include "packets.h"

#define TAS 1
#define BACK 2
#define MUTEX 3
#define ALOCK 4
#define CLH 5

#define LOCKFREE 1
#define HOME 2
#define RANDOM 3
#define LAST 4
#define AWESOME 5

volatile int go;

/* Get queue length
 */
int q_len(SerialList_t *q) {
  Item_t *curr = q->head;
  int count = 0;
  while (curr) {
    count++;
    curr = curr->next;
  }
  return count;
}


/* Keep time, set go flag to 0 after time is up */
void *timekeep(void *args)
{
  unsigned int time = *(unsigned int *) args;
  usleep(time*1000);
  go = 0;
  pthread_exit(NULL);
}


long serial_pack(unsigned int time,
		int n,
		long W,
		int uni,
		short exp)
{
  PacketSource_t * packetSource = createPacketSource(W, n, exp);

  StopWatch_t watch;
  int i, rc;
  long deqed = 0;
  go = 1;

  // Fingerprint destination
  long *fingerprint = (long *)malloc(sizeof(long)*n);
  for (i = 0; i < n; i++) {
    fingerprint[i] = 0;
  }

  // Spawn timer thread
  pthread_t timekeeper;
  if ((rc = pthread_create(&timekeeper, NULL, timekeep, &time))) {
    fprintf(stderr, "error: pthread_create, rc: %d\n", rc);
    exit(1);
  }

  if(uni) {
    startTimer(&watch);
    while(go) {
      for(i = 0; i < n; i++ ) {
	volatile Packet_t * tmp = getUniformPacket(packetSource,i);
	fingerprint[i] += getFingerprint(tmp->iterations, tmp->seed);
	deqed++;
      }
    }
    stopTimer(&watch);
  }
  else {
    startTimer(&watch);
    while(go) {
      for(i = 0; i < n; i++ ) {
	volatile Packet_t * tmp = getExponentialPacket(packetSource,i);
	fingerprint[i] += getFingerprint(tmp->iterations, tmp->seed);
	deqed++;
      }
    }
    stopTimer(&watch);
  }

  // Kill timekeeper
  pthread_join(timekeeper, NULL);
  
  //printf("%f\n",getElapsedTime(&watch));
  if (deqed < 0) {
    return 0;
  }
  return deqed;
}


void packet_spawn(int n, int type, int S, pthread_t *workers, pack_data_t *data) 
{
  int i, rc;
  
  switch(S) {
  case(LOCKFREE):
    for (i = 0; i < n; i++) {
      if ((rc = pthread_create(workers+i, NULL, lockfree, data+i))) {
	fprintf(stderr, "error: pthread_create, rc: %d\n", rc);
	exit(1);
      }
    }
    break;
  case(HOME):
    for (i = 0; i < n; i++) {
      if ((rc = pthread_create(workers+i, NULL, homeq, data+i))) {
	fprintf(stderr, "error: pthread_create, rc: %d\n", rc);
	exit(1);
      }
    }
    break;
  case(RANDOM):
    for (i = 0; i < n; i++) {
      if ((rc = pthread_create(workers+i, NULL, randomq, data+i))) {
	fprintf(stderr, "error: pthread_create, rc: %d\n", rc);
	exit(1);
      }
    }
    break;
  case(LAST):
    for (i = 0; i < n; i++) {
      if ((rc = pthread_create(workers+i, NULL, lastq, data+i))) {
	fprintf(stderr, "error: pthread_create, rc: %d\n", rc);
	exit(1);
      }
    }
    break;
  case(AWESOME):
    for (i = 0; i < n; i++) {
      if ((rc = pthread_create(workers+i, NULL, awesome, data+i))) {
	fprintf(stderr, "error: pthread_create, rc: %d\n", rc);
	exit(1);
      }
    }
  }
}
  

long parallel_pack(unsigned int time,
		  int n,
		  long W,
		  int uni,
		  short exp,
		  int D,
		  int type,
		  int S)
{
  PacketSource_t * packetSource = createPacketSource(W, n, exp);

  StopWatch_t watch;
  int i, j, rc;
  int enqed = 0;
  go = 1;

  // Create queues
  SerialList_t *queue[n];

  // Fingerprint destination
  long *fingerprint = (long *)malloc(sizeof(long)*n);
  for (i = 0; i < n; i++) {
    fingerprint[i] = 0;
    queue[i] = createSerialList();
  }

  // TAS args
  volatile int state[n];
  // MUTEX args
  pthread_mutex_t m[n];
  // Initialize alock
  volatile int anders[n][n*4]; 
  volatile long tail[n];
  volatile long head[n];
  // Initialize CLH tail
  volatile node_t *p[n];
 
  pack_data_t *data = (pack_data_t *)malloc(sizeof(pack_data_t)*n);
  //pack_data_t data[n];
  pthread_t workers[n];
  volatile lock_t *lock = (volatile lock_t *)malloc(sizeof(lock_t)*n);
  //volatile lock_t lock[n];
  // Or for clh
  volatile lock_t c_locks[n][n];

  // Initialize using switch over type
  switch (type) {
    
  case TAS:
    for (i = 0; i < n; i++) {
      lock[i].tas = state+i;
      state[i] = 0;
      data[i].lock_f = &tas_lock;
      data[i].unlock_f = &tas_unlock;
      data[i].try_f = &tas_try;
      data[i].locks = lock;
    }
    break;
  case BACK:
    for (i = 0; i < n; i++) {
      lock[i].tas = state+i;
      state[i] = 0;
      data[i].lock_f = &backoff_lock;
      data[i].unlock_f = &backoff_unlock;
      data[i].try_f = &backoff_try;
      data[i].locks = lock;
    }
    break;
  case MUTEX:
    for (i = 0; i < n; i++) {
      pthread_mutex_init(m+i, NULL);
      lock[i].m = m+i;
      data[i].lock_f = &mutex_lock;
      data[i].unlock_f = &mutex_unlock;
      data[i].try_f = &mutex_try;
      data[i].locks = lock;
    }
    break;
   case ALOCK:
    for (i = 0; i < n; i++) {
      anders[i][0] = 1;
      for (j = 1; j < n; j++) {
	anders[i][j*4] = 0;
      }
      lock[i].a.array = anders[i];
      tail[i] = 0;
      lock[i].a.tail = tail+i;
      lock[i].a.head = head+i;
      lock[i].a.max = n*4;
      
      data[i].lock_f = &anders_lock;
      data[i].unlock_f = &anders_unlock;
      data[i].try_f = &anders_try;
      data[i].locks = lock;
    }
    
    break;
  case CLH:
    for (i = 0; i < n; i++) {
      p[i] = new_clh_node();
      p[i]->locked = 0;
      data[i].lock_f = &clh_lock;
      data[i].unlock_f = &clh_unlock;
      data[i].try_f = &clh_try;
      data[i].locks = c_locks[i];
      for (j = 0; j < n; j++) {
	c_locks[i][j].clh.me = new_clh_node();
	c_locks[i][j].clh.tail = p+j;
      }
    }
  }  

  // Fill thread data 
  for (i = 0; i < n; i++) {
    data[i].id = i;
    data[i].n = n;
    data[i].my_count = 0;
    data[i].queue = queue;
    data[i].fingerprint = fingerprint;
  }

  // Spawn timer thread
  pthread_t timekeeper;
  if ((rc = pthread_create(&timekeeper, NULL, timekeep, &time))) {
    fprintf(stderr, "error: pthread_create, rc: %d\n", rc);
    exit(1);
  }

  // spawn worker threads
  packet_spawn(n, type, S, workers, data);
  
  if(uni) {
    startTimer(&watch);
    while(go) {
      for(i = 0; i < n; i++ ) {
	volatile Packet_t * tmp = getUniformPacket(packetSource,i);
	enqed += enqueue(queue[i], D, tmp, enqed);
      }
    }
    stopTimer(&watch);
  }
  else {
    startTimer(&watch);
    while(go) {
      for(i = 0; i < n; i++ ) {
	volatile Packet_t * tmp = getExponentialPacket(packetSource,i);
	enqed += enqueue(queue[i], D, tmp, enqed); 
      }
    }
    stopTimer(&watch);
  }

  // Kill timekeeper
  pthread_join(timekeeper, NULL);
  
  // Kill workers
  long deqed = 0;
  for (i = 0; i < n; i++) {
    pthread_join(workers[i], NULL);
    //printf("%i : %i...... from %i \n", i, data[i].my_count, enq[i]); 
    deqed += data[i].my_count;
    pthread_mutex_destroy(m+i);
  }
  //printf("Total = %i\nCounter = %i\nDiff = %i\n", enqed, deqed, enqed-deqed);

  //printf("%f\n",getElapsedTime(&watch));
  
  if (enqed-deqed < 0) {
    return 0;
  }
  return deqed;
}



void *lockfree(void *args) 
{
  pack_data_t *data = (pack_data_t *)args;
  SerialList_t **q = data->queue;
  long int *fp = data->fingerprint;
  int i = data->id;

  while(go) {
    (data->my_count) += dequeue(q[i], fp+i);
  }

  pthread_exit(NULL);
}



void *homeq(void *args) 
{
  pack_data_t *data = (pack_data_t *)args;
  SerialList_t **q = data->queue;
  long int *fp = data->fingerprint;
  volatile lock_t *locks = data->locks;
  void (*lockf)(volatile lock_t *) = data->lock_f;
  void (*unlockf)(volatile lock_t *) = data->unlock_f;
  int i = data->id;

  while(go) {
    (*lockf)(locks+i);
    (data->my_count) += dequeue(q[i], fp+i);
    (*unlockf)(locks+i);
  }
  pthread_exit(NULL);
}


void *randomq(void *args) 
{
  pack_data_t *data = (pack_data_t *)args;
  SerialList_t **q = data->queue;
  long int *fp = data->fingerprint;
  volatile lock_t *locks = data->locks;
  void (*lockf)(volatile lock_t *) = data->lock_f;
  void (*unlockf)(volatile lock_t *) = data->unlock_f;
  int i;

  while(go) {
    i = rand() % data->n;
    (*lockf)(locks+i);
    (data->my_count) += dequeue(q[i], fp+i);
    (*unlockf)(locks+i);
  }
  pthread_exit(NULL);
}


void *lastq(void *args)
{
  pack_data_t *data = (pack_data_t *)args;
  SerialList_t **q = data->queue;
  long int *fp = data->fingerprint;
  volatile lock_t *locks = data->locks;
  void (*unlockf)(volatile lock_t *) = data->unlock_f;
  int (*tryf)(volatile lock_t *) = data->try_f;
  int i = rand() % data->n;


  while(go) {
    while((*tryf)(locks+i)) {
      i = rand() % data->n;
    }
    (data->my_count) += dequeue(q[i], fp+i);
    (*unlockf)(locks+i);
  }
  pthread_exit(NULL);
}


void *awesome(void *args)
{
  pack_data_t *data = (pack_data_t *)args;
  SerialList_t **q = data->queue;
  long int *fp = data->fingerprint;
  int i;

  while(go) {
    i = rand() % data->n;
    (data->my_count) += dequeue(q[i], fp+i);
  }
  pthread_exit(NULL);
}


int dequeue(SerialList_t *q, long int *fingerprint) 
{
  Item_t *curr = q->head;
  Item_t *prev = NULL;
  volatile Packet_t *tmp;
  if(!curr || !q) {
    return 0;
  }
  if (q->head != q->tail) {
     while (curr->next) {
      prev = curr;
      curr = curr->next;
    }
    tmp = curr->value;
    *fingerprint += getFingerprint(tmp->iterations, tmp->seed);
    remove_list(q, curr->key);
    (q->tail) = prev;
    return 1;
  }
  return 0;
}


int enqueue(SerialList_t *q, int D, volatile Packet_t *packet, int key) {
  if (q_len(q) == D) {
    return 0;
  }    
  add_list(q, key, packet);
  return 1;
}

