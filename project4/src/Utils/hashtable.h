#ifndef HASHTABLE_H_
#define HASHTABLE_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

#include "generators.h"
#include "stopwatch.h"
#include "fingerprint.h"
#include "packetsource.h"
#include "seriallist.h"
#include <stdbool.h>

#define SERIAL 0
#define LOCKED 1
#define LOCKFREEC 2
#define LINEARPROBED 3
#define AWESOME 4

typedef struct serialTable_t {
  int logSize;
  int mask;
  int maxBucketSize;
  int size;
  SerialList_t ** table;
}serialTable_t;

typedef struct lockedTable_t { 
  int logSize;
  int mask;
  int maxBucketSize;
  int size;
  SerialList_t ** table;
  pthread_rwlock_t *rw_locks;
} lockedTable_t;

typedef struct lockFreeCTable_t {
  int logSize;
  int mask;
  int maxBucketSize;
  int size;
  SerialList_t ** table;
  pthread_mutex_t *locks;
} lockFreeCTable_t;

typedef struct linearProbedTable_t {
  int logSize;
  int mask;
  int maxBucketSize;
  int size;
} linearProbedTable_t;

typedef struct awesomeTable_t {
  int logSize;
  int mask;
  int maxBucketSize;
  int size;
} awesomeTable_t;

typedef union hashtable_t {
  serialTable_t *serial;
  lockedTable_t *locked;
  lockFreeCTable_t *lockFreeC;
  linearProbedTable_t *linearProbed;
  awesomeTable_t *awesome;
}hashable_t;

/* For serial hash table */
serialTable_t * createserialTable(int logSize, int maxBucketSize);
void resizeIfNecessary_ht(serialTable_t * htable,int key);
void addNoCheck_ht(serialTable_t * htable,int key, volatile Packet_t * x);
void add_ht(serialTable_t * htable,int key, volatile Packet_t * x);
bool remove_ht(serialTable_t * htable,int key);
bool contains_ht(serialTable_t * htable,int key);
void resize_ht(serialTable_t * htable);
void print_ht(serialTable_t * htable);




#endif /* HASHTABLE_H_ */
