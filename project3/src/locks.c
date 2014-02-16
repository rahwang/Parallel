#include "locks.h"

#define MIN_DELAY 1
#define MAX_DELAY 1000



/* TAS lock functions */
void tas_lock(volatile lock_t *lock) 
{
  while (__sync_lock_test_and_set(lock->tas, 1)) {}
}

void tas_unlock(volatile lock_t *lock) 
{
  //__sync_lock_test_and_set(lock->tas, 0);
  *(lock->tas) = 0;
}



/* Exponential Backoff Lock functions */
void backoff_lock(volatile lock_t *lock) 
{
  int time;
  double backoff = 0;

  while(1) {
    if (!__sync_lock_test_and_set(lock->tas, 1)) {
      return;
    } 
    else {
      time = rand()*fmax(pow(2, backoff), MAX_DELAY);
      backoff++;
      usleep(time);
    }
  }
}

void backoff_unlock(volatile lock_t *lock) 
{
  //__sync_lock_test_and_set(lock->tas, 0);
  *(lock->tas) = 0;
}



/* Mutex lock functions */
void mutex_lock(volatile lock_t *lock) 
{
  pthread_mutex_lock(lock->mutex);
}

void mutex_unlock(volatile lock_t *lock) 
{
  pthread_mutex_unlock(lock->mutex);
}



/* Anderson lock functions */
void anders_lock(volatile lock_t *lock) 
{
  alock_t *a = lock->alock;
  int idx = __sync_fetch_and_add(a->tail, 4) % a->max;
  while(!(a->array)[idx]) {}
}

void anders_unlock(volatile lock_t *lock)
{
  alock_t *a = lock->alock;
  (a->array)[*(a->head)] = 0;
  *(a->head) += 4 % a->max;
  (a->array)[*(a->head)] = 1;
}



/* CLH lock functions */
node_t *new_clh_node()
{
  return (node_t *)malloc(sizeof(node_t));
}

void clh_lock(volatile lock_t *lock) 
{
  clh_t *c = &(lock->clh);
  volatile node_t *curr = c->me;
  //__sync_lock_test_and_set(&(curr->locked), 1);
  curr->locked = 1;
  c->pred = __sync_lock_test_and_set(*(c->tail), curr);
  while ((c->pred)->locked) {}
}

void clh_unlock(volatile lock_t *lock) 
{
  clh_t *c = lock->clh;
  //__sync_lock_test_and_set(&(curr->locked), 0);
  (c->me)->locked = 0;
  c->me = c->pred;
}
