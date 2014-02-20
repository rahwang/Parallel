#ifndef WORK_COUNTER_H
#define WORK_COUNTER_H

#include "locks.h"

/* From serial_work_counter.c */
void *s_work_worker();
double serial_work(int work);

/* From parallel_work_counter.c */
void *tas(void *arg);
void *back(void *arg);
void *mutex(void *arg);
void *anders(void *arg);
void *clh(void *arg);
void spawn_work(int type, 
		      int n, 
		      pthread_t *workers,
		      thr_data_t *data);
double parallel_work(int work, int n, int type);

#endif
