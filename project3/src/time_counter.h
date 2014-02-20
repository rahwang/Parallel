
#ifndef TIME_COUNTER_H
#define TIME_COUNTER_H

#include "locks.h"

/* From serial_time_counter.c */
void *s_time_worker();
long serial_time(unsigned int time);

/* From parallel_time_counter.c */
void *tas(void *arg);
void *back(void *arg);
void *mutex(void *arg);
void *anders(void *arg);
void *clh(void *arg);
void spawn_time(int type, 
		int n, 
		pthread_t *workers,
		thr_data_t *data);
long parallel_time(unsigned int time, int n, int type, int exp);

#endif
