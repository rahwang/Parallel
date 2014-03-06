
#include "hashpacketworker.h"
#include "fingerprint.h"
#include "hashtable.h"
#include "paddedprim.h"

void serialWorker(SerialPacketWorker_t * data){
  HashPacket_t * pkt;
  
  while( !data->done->value ) {
    data->totalPackets++;
    pkt = getRandomPacket(data->source);
    data->residue += getFingerprint(pkt->body->iterations, pkt->body->seed);
    
    switch(pkt->type) {
    case Add:
      add_serial(data->table,mangleKey(pkt),pkt->body);
      break;
    case Remove:
      remove_serial(data->table,mangleKey(pkt));
      break;
    case Contains:
      contains_serial(data->table,mangleKey(pkt));
      break;
    }
  }
}


void parallelWorker(ParallelPacketWorker_t * data){
  volatile HashPacket_t * pkt;
  hashtable_t *table = data->table;
  HashList_t **queues = data->queues;
  int i = data->tid;
  
  void (*addf)(hashtable_t *, int, volatile Packet_t *) = data->addf;
  bool (*removef)(hashtable_t *, int) = data->removef;
  bool (*containsf)(hashtable_t *, int) = data->containsf;
  
  while( !data->done->value ) {
    data->totalPackets++;
    pkt = getPacket(queues, i);
    data->fingerprints += getFingerprint(pkt->body->iterations, pkt->body->seed);
    
    switch(pkt->type) {
    case Add:
      (*addf)(table, mangleKey((HashPacket_t *)pkt), pkt->body);
      break;
    case Remove:
      (*removef)(table, mangleKey((HashPacket_t *)pkt));
      break;
    case Contains:
      (*containsf)(table, mangleKey((HashPacket_t *)pkt));
      break;
    }
  }
}

/* Get queue length
 */
int q_len(HashList_t *q) {
  HashItem_t *curr = q->head;
  int count = 0;
  while (curr) {
    count++;
    curr = curr->next;
  }
  return count;
}

int enqueue(HashList_t *q, int D, volatile HashPacket_t *packet, int key) {
  if (q_len(q) == D) {
    return 0;
  }    
  add_hashlist(q, key, packet);
  return 1;
}

volatile HashPacket_t *getPacket(HashList_t **q, int i) 
{
  volatile HashPacket_t *pkt;
  while (!(pkt = dequeue(q[i]))) {};
  return pkt;
}
  
volatile HashPacket_t *dequeue(HashList_t *q) 
{
  HashItem_t *curr = q->head;
  HashItem_t *prev = NULL;
  volatile HashPacket_t *tmp;
  if(!curr || !q) {
    return NULL;
  }
  if (q->head != q->tail) {
     while (curr->next) {
      prev = curr;
      curr = curr->next;
    }
    tmp = curr->value;
    remove_hashlist(q, curr->key);
    (q->tail) = prev;
    return tmp;
  }
  return NULL;
}
