#include "locks.h"

#define MIN_DELAY 1
#define MAX_DELAY 1000



/* TAS lock functions */
void tas_lock(volatile int *state) 
{
  while (__sync_lock_test_and_set(state, 1)) {}
}

void tas_unlock(volatile int *state) 
{
  __sync_lock_test_and_set(state, 0);
}



/* Exponential Backoff Lock functions */
void backoff_lock(volatile int *state, volatile int *backoff) 
{
  int time;

  while(1) {
    while(*state) {};
    if (!__sync_lock_test_and_set(state, 1)) {
      return;
    } 
    else {
      time = rand()*fmax(pow(2, (double)*backoff), MAX_DELAY);
      (*backoff)++;
      nanosleep((struct timespec[]){{0, 1000000*time}}, NULL);
    }
  }
}

void backoff_unlock(volatile int *state) 
{
  __sync_lock_test_and_set(state, 0);
}



/* Mutex lock functions */
void mutex_lock(pthread_mutex_t *m) 
{
  pthread_mutex_lock(m);
}

void mutex_unlock(pthread_mutex_t *m) 
{
  pthread_mutex_unlock(m);
}



/* Anderson lock functions */
void anders_lock(volatile alock_t *a, volatile int *idx) 
{
  int curr = __sync_fetch_and_add(a->tail, 4) % a->max;
  while(1) 
    {
      if (__sync_lock_test_and_set((a->array)+curr, 1)) {
	*idx = curr; 
	return;
      }
    }
}

void anders_unlock(volatile alock_t *a, volatile int *idx)
{
  __sync_lock_test_and_set((a->array)+(*idx), 0);
}



/* CLH lock functions */
node_t *new_clh_node()
{
  return (node_t *)malloc(sizeof(node_t));
}

void clh_lock(volatile clh_t *lock) 
{
  volatile node_t *curr = lock->me;
  //__sync_lock_test_and_set(&(curr->locked), 1);
  curr->locked = 1;
  lock->pred = __sync_lock_test_and_set((lock->tail), curr);
  while ((lock->pred)->locked) {}
}

void clh_unlock(volatile clh_t *lock) 
{
  volatile node_t *curr = lock->me;
  //__sync_lock_test_and_set(&(curr->locked), 0);
  curr->locked = 0;
  lock->me = lock->pred;
}
