 #include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "Utils/generators.h"
#include "Utils/stopwatch.h"
#include "Utils/fingerprint.h"
#include "Utils/packetsource.h"

#define DEFAULT_NUMBER_OF_ARGS 6

// you can do it Rachel!!

void serialFirewall(const int,
		    const int,
		    const long,
		    const int,
		    const short);


void dequeue(Seriallist_t *q, int *fingerprint) {
  while(q->size > 0) {
    Item_t *curr = q->head;
    Packet_t *tmp = curr->value;
    fingerprint += getFingerprint(tmp->iterations, tmp->seed);
    remove_list(q, curr->key);
  }
}


int enqueue(int *count, Seriallist_t *q, int numPackets, int D, PacketSource_t *packetSource) {
  while (*count < numPackets) {
    if (q->size == D) {
      break;
    }
    volatile Packet_t *tmp = getUniformPacket(packetSource,i);
    add_list(q, *count, tmp);
    (*count)++;
  }
  if (*count == numPackets) {
    (*count)++;
    return 1;
  }
  return 0;
}


int main(int argc, char * argv[]) {

	if(argc >= DEFAULT_NUMBER_OF_ARGS) {
        const int numPackets = atoi(argv[1]);
		const int numSources = atoi(argv[2]);
		const long mean = atol(argv[3]);
		const int uniformFlag = atoi(argv[4]);
		const short experimentNumber = (short)atoi(argv[5]);

        serial_queue_firewall(numPackets,numSources,mean,uniformFlag,experimentNumber);
	}
    return 0;
}
void serial_queue_firewall (int numPackets,
					 int numSources,
					 long mean,
					 int uniformFlag,
					 short experimentNumber)
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
	       if (count[i] < numPackets) {
		 done += enqueue(count[i], queues[i], numPackets, D, packetSource);
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
	       enqueue(count[i], queues[i], done, numPackets, D, packetSource);
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
