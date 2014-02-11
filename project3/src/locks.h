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
  int max;
} alock_t;


typedef struct node_t{
  volatile int locked;
} node_t;


typedef struct clh_t{
  volatile node_t *me;
  volatile node_t *pred;
  volatile node_t **tail;
} clh_t;


typedef struct thr_data_t{
  volatile int *state;
  volatile int *backoff;
  volatile int *counter;
  pthread_mutex_t *mutex;
  volatile alock_t *alock;
  volatile node_t **clh_tail;
  volatile int my_count;
} thr_data_t;


void tas_lock(volatile int *state);
void tas_unlock(volatile int *state);

void backoff_lock(volatile int *state, volatile int *backoff);
void backoff_unlock(volatile int *state);

void mutex_lock(pthread_mutex_t *m);
void mutex_unlock(pthread_mutex_t *m);

void anders_lock(volatile alock_t *a, volatile int *idx);
void anders_unlock(volatile alock_t *a, volatile int *idx);

node_t *new_clh_node();
void clh_lock(volatile clh_t *lock);
void clh_unlock(volatile clh_t *lock);

#endif
