#ifndef PACKETS_H
#define PACKETS_H

#include "locks.h"

int q_len(SerialList_t *q);
void *timekeep(void *args);
int dequeue(SerialList_t *q, long int *fingerprint);
int enqueue(SerialList_t *q, int D, volatile Packet_t *packet, int key);

long *serial_pack(unsigned int time,
		  int n,
		  long W,
		  int uni,
		  short exp);

long *parallel_pack(unsigned int time,
		    int n,
		    long W,
		    int uni,
		    short exp,
		    int D,
		    int type,
		    int S);

#endif
