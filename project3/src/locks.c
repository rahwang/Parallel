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
  *(lock->tas) = 0;
}

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
/*
int anders_try(volatile lock_t *lock) 
{
 
}
*/
/* CLH lock functions */
node_t *new_clh_node()
{
  return (node_t *)malloc(sizeof(node_t));
}

void clh_lock(volatile lock_t *lock) 
{
  //clh_t c = lock->clh;
  volatile node_t *curr = (lock->clh).me;
  curr->locked = 1;
  (lock->clh).pred = __sync_lock_test_and_set(((lock->clh).tail), curr);
  while (((lock->clh).pred)->locked) {}
}

void clh_unlock(volatile lock_t *lock) 
{
  //clh_t c = lock->clh;
  ((lock->clh).me)->locked = 0;
  (lock->clh).me = (lock->clh).pred;
}
