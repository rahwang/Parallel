#include "packets.h"

#define DONE 1000000

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


long *serial_pack(unsigned int time,
		  int n,
		  long W,
		  int uni,
		  short exp)
{
  PacketSource_t * packetSource = createPacketSource(W, n, exp);

  StopWatch_t watch;
  int i, rc;
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
      }
    }
    stopTimer(&watch);
  }

  // Kill timekeeper
  pthread_join(timekeeper, NULL);
  
  //printf("%f\n",getElapsedTime(&watch));
  return fingerprint;
}


long *parallel_pack(unsigned int time,
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
  int i, rc;
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

  // Spawn timer thread
  pthread_t timekeeper;
  if ((rc = pthread_create(&timekeeper, NULL, timekeep, &time))) {
    fprintf(stderr, "error: pthread_create, rc: %d\n", rc);
    exit(1);
  }

  // spawn worker threads


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
  
  //printf("%f\n",getElapsedTime(&watch));
  return fingerprint;
}


int dequeue(SerialList_t *q, long int *fingerprint) {
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
    q->tail = prev;
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

