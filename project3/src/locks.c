#include "locks.h"

#define MIN_DELAY 1
#define MAX_DELAY 256



/* TAS lock functions */
void tas_lock(volatile lock_t *lock) 
{
  while (__sync_lock_test_and_set(lock->tas, 1)) {}
}

void tas_unlock(volatile lock_t *lock) 
{
  *(lock->tas) = 0;
}

// Attempts to acquire lock. Returns 0 on success.
int tas_try(volatile lock_t *lock)
{ 
  return __sync_lock_test_and_set(lock->tas, 1);
}



/* Exponential Backoff Lock functions */
void backoff_lock(volatile lock_t *lock) 
{
  double time;
  double backoff = 0;

  while(1) {
    if (!__sync_lock_test_and_set(lock->tas, 1)) {
      return;
    } 
    else {    
      time = fmin((rand()/(double)RAND_MAX)*pow(2, backoff), MAX_DELAY);
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

// Attempts to acquire lock. Returns 0 on success.
int backoff_try(volatile lock_t *lock)
{ 
  return __sync_lock_test_and_set(lock->tas, 1);
}



/* Mutex lock functions */
void mutex_lock(volatile lock_t *lock) 
{
  pthread_mutex_lock(lock->m);
}

void mutex_unlock(volatile lock_t *lock) 
{
  pthread_mutex_unlock(lock->m);
}

// Attempts to acquire lock. Returns 0 on success.
int mutex_try(volatile lock_t *lock) 
{
  return pthread_mutex_trylock(lock->m);
}



/* Anderson lock functions */
void anders_lock(volatile lock_t *lock) 
{
  //alock_t a = lock->a;
  int idx = __sync_fetch_and_add((lock->a).tail, 4) % (lock->a).max;
  while(!((lock->a).array)[idx]) {}
  *((lock->a).head) = idx;
}

void anders_unlock(volatile lock_t *lock)
{
  int idx = *((lock->a).head);
  ((lock->a).array)[idx] = 0;
  ((lock->a).array)[(idx + 4) % (lock->a).max] = 1;
}

// Attempts to acquire lock. Returns 0 on success.
int anders_try(volatile lock_t *lock) 
{
  //alock_t a = lock->a;
  int max = (lock->a).max;
  volatile long *tail = (lock->a).tail;

  if ((lock->a).array[*tail % max]) {
    anders_lock(lock);
    return 0;
  } 
  return 1;
}


/* CLH lock functions */
node_t *new_clh_node()
{
  return (node_t *)malloc(sizeof(node_t));
}

void clh_lock(volatile lock_t *lock) 
{
  //clh_t c = lock->clh;
  //volatile node_t *curr = (lock->clh).me;
  (lock->clh).me->locked = 1;
  (lock->clh).pred = __sync_lock_test_and_set((lock->clh).tail, (lock->clh).me);
  while (((lock->clh).pred)->locked) {}
}

void clh_unlock(volatile lock_t *lock) 
{
  //clh_t c = lock->clh;
  volatile node_t *tmp = (lock->clh).pred;
  ((lock->clh).me)->locked = 0;
  (lock->clh).me = tmp;
  //(lock->clh).me = (lock->clh).pred;
}

int clh_try(volatile lock_t *lock)
{
  if ((*((lock->clh).tail))->locked) {
    return 1;
  }
  clh_lock(lock);
  return 0;
}
