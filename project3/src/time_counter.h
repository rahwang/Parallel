#ifndef TIME_COUNTER_H
#define TIME_COUNTER_H

/* From serial_time_counter.c */
void *increment();
int serial_time(int time);

/* From parallel_time_counter.c */
void *tas(void *arg);
void *back(void *arg);
void *mutex(void *arg);
void *anders(void *arg);
void *clh(void *arg);
pthread_t *spawn_time(int type, 
		      int n, 
		      volatile int *counter, 
		      volatile int *state, 
		      volatile int *backoff, 
		      pthread_mutex_t *m, 
		      volatile alock_t *alock, 
		      volatile node_t **clh_tail, 
		      thr_data_t *data);
int parallel_time(int time, int n, int type);

#endif
