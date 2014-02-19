#ifndef PACKETS_H
#define PACKETS_H

#include "locks.h"

int q_len(SerialList_t *q);
void *timekeep(void *args);
int dequeue(SerialList_t *q, long int *fingerprint);
int enqueue(SerialList_t *q, int D, volatile Packet_t *packet, int key);
void packet_spawn(int n, int type, int S, pthread_t *workers, pack_data_t *data);
void *lockfree(void *args);
void *homeq(void *args); 
void *randomq(void *args); 
void *lastq(void *args); 
void *awesome(void *args);
 
long *serial_pack(unsigned int time,
		int n,
		long W,
		int uni,
		short exp);

pack_data_t *parallel_pack(unsigned int time,
		    int n,
		    long W,
		    int uni,
		    short exp,
		    int D,
		    int type,
		    int S);

#endif
