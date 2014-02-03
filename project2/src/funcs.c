#include "funcs.h"

void serial_firewall (int numPackets,
		      int numSources,
		      long mean,
		      int uniformFlag,
		      short experimentNumber,
		      int queueDepth)
{
  PacketSource_t * packetSource = createPacketSource(mean, numSources, experimentNumber);
  StopWatch_t watch;
  long fingerprint = 0;
  int i, j;
  
  if( uniformFlag) {
    startTimer(&watch);
    for(i = 0; i < numSources; i++ ) {
      for(j = 0; j < numPackets; j++ ) {
	volatile Packet_t * tmp = getUniformPacket(packetSource,i);
	fingerprint += getFingerprint(tmp->iterations, tmp->seed);
      }
    }
    stopTimer(&watch);
  }
  else {
    startTimer(&watch);
    for(i = 0; i < numSources; i++ ) {
      for(j = 0; j < numPackets; j++ ) {
	volatile Packet_t * tmp = getExponentialPacket(packetSource,i);
	fingerprint += getFingerprint(tmp->iterations, tmp->seed);
      }
    }
    stopTimer(&watch);
  }
  printf("%f\n",getElapsedTime(&watch));
}


void dequeue(SerialList_t *q, long int *fingerprint) {
  while(q->size > 0) {
    Item_t *curr = q->head;
    while (curr->next) {
      curr = curr->next;
    }
    volatile Packet_t *tmp = curr->value;
    *fingerprint += getFingerprint(tmp->iterations, tmp->seed);
    remove_list(q, curr->key);
  }
}


int enqueue(int *count, SerialList_t *q, int numPackets, int depth, volatile Packet_t *packet) {
  if (q->size == depth) {
    return 0;
  }
  add_list(q, *count, packet);
  (*count)++;

  if (*count == numPackets) {
    (*count)++;
    return 1;
  }
  return 0;
}


SerialList_t **create_queues(int numSources) {
  int i;
  SerialList_t **new = (SerialList_t **)malloc(sizeof(SerialList_t *)*numSources);
  for (i = 0; i < numSources; i++) {
    new[i] = createSerialList();
  }
  return new;
}


void serial_queue_firewall (int numPackets,
			    int numSources,
			    long mean,
			    int uniformFlag,
			    short experimentNumber,
			    int queueDepth)
{
  PacketSource_t * packetSource = createPacketSource(mean, numSources, experimentNumber);
  StopWatch_t watch;
  int i;
  
  // Create queues
  SerialList_t **queues = create_queues(numSources);
  
  // Store number of packets already enqueued, per queue
  int *count = (int *)malloc(sizeof(int)*numSources);
  for (i = 0; i < numSources; i++) {
    count[i] = 0;
  }
  
  // Fingerprint destination
  long *fingerprint = (long *)malloc(sizeof(long)*numSources);
  for (i = 0; i < numSources; i++) {
    fingerprint[i] = 0;
  }
  
  // Number of sources finished
  int done = 0;
  
  // Uniform case
  if(uniformFlag) {
    startTimer(&watch);
    
    // Enqueue while not all packets have been enqueued
    while (done < numSources) {
      for( i = 0; i < numSources; i++ ) {
	if ((count[i] < numPackets) && (queues[i]->size < queueDepth)) {
	  volatile Packet_t *packet = getUniformPacket(packetSource,i);
	  done += enqueue(count+i, queues[i], numPackets, queueDepth, packet);
	}
      }
      // Dequeue in each queue
      for( i = 0; i < numSources; i++ ) {
	dequeue(queues[i], fingerprint+i);
      }
    }       
    stopTimer(&watch);
    
    // Non-uniform case
  } else {
    startTimer(&watch);
    
    // Enqueue while not all packets have been enqueued
    while (done < numSources) {
      for( i = 0; i < numSources; i++ ) {
	if ((count[i] < numPackets) && (queues[i]->size < queueDepth)) {
	  volatile Packet_t *packet = getExponentialPacket(packetSource,i);
	  enqueue(count+i, queues[i], numPackets, queueDepth, packet);
	}
      }
      // Dequeue in each queue
      for( i = 0; i < numSources; i++ ) {
	dequeue(queues[i], fingerprint+i);
      }
    }       
    stopTimer(&watch);
  }
  printf("%f\n",getElapsedTime(&watch));
}

