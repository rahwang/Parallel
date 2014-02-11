#ifndef WORK_COUNTER_H
#define WORK_COUNTER_H

/* From serial_work_counter.c */
void *increment();
int serial_work(int work);

/* From parallel_work_counter.c */
void *tas(void *arg);
void *back(void *arg);
void *mutex(void *arg);
void *anders(void *arg);
void *clh(void *arg);
pthread_t *spawn_work(int type, 
		      int n, 
		      int work,
		      volatile int *counter, 
		      volatile int *state, 
		      volatile int *backoff, 
		      pthread_mutex_t *m, 
		      volatile alock_t *alock, 
		      volatile node_t **clh_tail, 
		      thr_data_t *data);
int parallel_work(int work, int n, int type);

#endif
