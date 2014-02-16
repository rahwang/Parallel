#ifndef LOCKS_H
#define LOCKS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

#include "Utils/generators.h"
#include "Utils/stopwatch.h"
#include "Utils/fingerprint.h"
#include "Utils/packetsource.h"
#include "Utils/seriallist.h"

typedef struct alock_t{
  volatile int *array;
  volatile int *tail;
  volatile int *head;
  int max;  
} alock_t;

typedef struct node_t{
  volatile int locked;
  volatile struct node_t *pred;
} node_t;

typedef struct clh_t{
  volatile node_t *me;
  volatile node_t *pred;
  volatile node_t **tail;
} clh_t;

typedef volatile int *tas_t;

typedef volatile pthread_mutex_t *mutex_t;

typedef union lock_t {
  mutex_t mutex;
  tas_t tas;
  clh_t clh;
  alock_t alock;
} lock_t;

typedef struct thr_data_t{
  volatile int *counter;
  volatile lock_t *locks;
  volatile int my_count;
  void (*lock_f) (lock_t *);
  void (*unlock_f) (lock_t *);
} thr_data_t;

void tas_lock(volatile lock_t *lock);
void tas_unlock(volatile lock_t *lock);

void backoff_lock(volatile lock_t *lock);
void backoff_unlock(volatile lock_t *lock);

void mutex_lock(volatile lock_t *lock);
void mutex_unlock(volatile lock_t *lock);

void anders_lock(volatile lock_t *lock);
void anders_unlock(volatile lock_t *lock);

node_t *new_clh_node();
void clh_lock(volatile lock_t *lock);
void clh_unlock(volatile lock_t *lock);

#endif
