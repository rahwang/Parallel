#ifndef FUNCS_H
#define FUNCS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "Utils/generators.h"
#include "Utils/stopwatch.h"
#include "Utils/fingerprint.h"
#include "Utils/packetsource.h"
#include "Utils/seriallist.h"

/* Run serial-queue firewall */
void serial_queue_firewall (int numPackets,
			    int numSources,
			    long mean,
			    int uniformFlag,
			    short experimentNumber,
			    int queueDepth);

/* Run serial firewall */
void serial_firewall (int numPackets,
		      int numSources,
		      long mean,
		      int uniformFlag,
		      short experimentNumber,
		      int queueDepth);

/* Run parallel firewall */
void parallel_firewall (int numPackets,
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

#endif
