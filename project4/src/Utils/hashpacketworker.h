#ifndef HASHPACKETWORKER_H_
#define HASHPACKETWORKER_H_

#include "paddedprim.h"
#include "hashgenerator.h"
#include "hashtable.h"

typedef struct SerialPacketWorker_t{
  volatile int *go;
  HashPacketGenerator_t * source;
  serialTable_t * table;
  long totalPackets;
  long residue;
  long fingerprint;
} SerialPacketWorker_t;

typedef struct ParallelPacketWorker_t{
  volatile int *go;
  hashtable_t * table;
  //volatile long totalPackets;
  HashList_t **queues;
  pthread_mutex_t *locks;
  volatile int tid;
  volatile int n;
  volatile long myCount;
  void (*addf) (hashtable_t *, int, volatile Packet_t *);
  bool (*removef) (hashtable_t *, int);
  bool (*containsf) (hashtable_t *, int);
  long *fingerprints;
} ParallelPacketWorker_t;

void serialWorker(SerialPacketWorker_t *data);
void parallelWorker(ParallelPacketWorker_t *data);
void noloadWorker(ParallelPacketWorker_t *data);

int enqueue(HashList_t *q, int D, volatile HashPacket_t *packet, int key);
volatile HashPacket_t *getPacket(HashList_t **q, pthread_mutex_t *locks, int id, int n);
volatile HashPacket_t *dequeue(HashList_t *q); 

#endif /* HASHPACKETWORKER_H_ */
