#ifndef FUNCS_H
#define FUNCS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "Utils/generators.h"
#include "Utils/stopwatch.h"
#include "Utils/fingerprint.h"
#include "Utils/packetsource.h"
#include "Utils/seriallist.h"

/* Run serial-queue firewall */
long *serial_queue_firewall (int numPackets,
			    int numSources,
			    long mean,
			    int uniformFlag,
			    short experimentNumber,
			    int queueDepth);

/* Run serial firewall */
long *serial_firewall (int numPackets,
		      int numSources,
		      long mean,
		      int uniformFlag,
		      short experimentNumber,
		      int queueDepth);

/* Run parallel firewall */
long *parallel_firewall (int numPackets,
			int numSources,
			long mean,
			int uniformFlag,
			short experimentNumber,
			int queueDepth);

/* Calculate packet then remove it from queue */
void dequeue(SerialList_t *q, long int *fingerprint);

/* Generate packet and place in specified queue */
int enqueue(int *count, SerialList_t *q, int numPackets, int depth, volatile Packet_t *packet);

/* Allocate queues */
SerialList_t **create_queues(int numSources);

/* define thread data struct */
typedef struct thr_data_t {
  int *count;
  long int *fp;
  int tid;
  SerialList_t *q;
} thr_data_t;

/* Spawn threads */
pthread_t *thread(int n, int *count, SerialList_t **queues, long int *fingerprint);

/* Thread function: dequeue */
void *thr_dequeue(void *arg);

/* Declare barrier */
extern pthread_barrier_t b; 

#endif
