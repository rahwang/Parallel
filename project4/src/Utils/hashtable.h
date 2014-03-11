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
#include "hashlist.h"
#include <stdbool.h>

#define SERIAL 0
#define LOCKED 1
#define LOCKFREEC 2
#define LINEARPROBED 3
#define AWESOME 4

typedef struct serialTable_t {
  volatile int logSize;
  volatile int mask;
  volatile int maxBucketSize;
  volatile int size;
  SerialList_t ** table;
}serialTable_t;

typedef struct lockedTable_t { 
  volatile int logSize;
  volatile int mask;
  volatile int maxBucketSize;
  volatile int size;
  volatile int *entries;
  SerialList_t ** table;
  volatile int numLocks;
  pthread_rwlock_t *rw_locks;
} lockedTable_t;

typedef struct lockFreeCTable_t {
  volatile int logSize;
  volatile int mask;
  volatile int maxBucketSize;
  volatile int size;
  volatile int *entries;
  SerialList_t ** table;
  volatile int numLocks;
  pthread_mutex_t *locks;
} lockFreeCTable_t;

typedef struct Pack_t {
  int key;
  volatile Packet_t * value;
} Pack_t;

typedef struct linearProbeTable_t {
  volatile int logSize;
  volatile int mask;
  volatile int maxStep;
  volatile int owned;
  volatile int *entries;
  volatile int size;
  Pack_t *table;
  volatile int numLocks;
  pthread_mutex_t *locks;
} linearProbeTable_t;

typedef struct node_t {
  volatile Packet_t *val;
  int key;
  struct node_t *next;
  int marked;
} node_t;

typedef struct window {
  node_t *curr;
  node_t *pred;
} window;

typedef struct bucketlist_t {
  node_t *head;
} bucketlist_t;

typedef struct awesomeTable_t {
  volatile long setSize;
  volatile long bucketSize;
  volatile long maxSize;
  bucketlist_t *buckets;
} awesomeTable_t;

typedef union hashtable_t {
  lockedTable_t *locked;
  lockFreeCTable_t *lockFreeC;
  linearProbeTable_t *linearProbe;
  awesomeTable_t *awesome;
}hashtable_t;

/* For serial hash table */
serialTable_t * createSerialTable(int logSize, int maxBucketSize);
void resizeIfNecessary_serial(serialTable_t * htable,int key);
void addNoCheck_serial(serialTable_t * htable,int key, volatile Packet_t * x);
void add_serial(serialTable_t * htable,int key, volatile Packet_t * x);
bool remove_serial(serialTable_t * htable,int key);
bool contains_serial(serialTable_t * htable,int key);
void resize_serial(serialTable_t * htable);
void print_serial(serialTable_t * htable);
void free_serial(serialTable_t *table);

/* For locked hash table */
lockedTable_t * createLockedTable(int logSize, int maxBucketSize, int n);
void resizeIfNecessary_locked(lockedTable_t * table,int key);
void addNoCheck_locked(lockedTable_t * table,int key, volatile Packet_t * x);
void add_locked(hashtable_t * htable,int key, volatile Packet_t * x);
bool remove_locked(hashtable_t * htable,int key);
bool contains_locked(hashtable_t * htable,int key);
void resize_locked(lockedTable_t * table);
void print_locked(lockedTable_t * table);

/* For lock-free-contains hash table */
lockFreeCTable_t * createLockFreeCTable(int logSize, int maxBucketSize, int n);
void resizeIfNecessary_lockFreeC(lockFreeCTable_t * htable,int key);
void addNoCheck_lockFreeC(lockFreeCTable_t * htable,int key, volatile Packet_t * x);
void add_lockFreeC(hashtable_t * htable,int key, volatile Packet_t * x);
bool remove_lockFreeC(hashtable_t * htable,int key);
bool contains_lockFreeC(hashtable_t * htable,int key);
void resize_lockFreeC(lockFreeCTable_t * htable);
void print_lockFreeC(lockFreeCTable_t * htable);

/* For lock free C hash table */
linearProbeTable_t * createLinearProbeTable(int logSize, int maxStep, int n);
void resizeIfNecessary_linearProbe(linearProbeTable_t * htable,int key);
void add_linearProbe(hashtable_t * htable,int key, volatile Packet_t * x);
bool remove_linearProbe(hashtable_t * htable,int key);
bool contains_linearProbe(hashtable_t * htable,int key);
void resize_linearProbe(linearProbeTable_t * htable);
void print_linearProbe(linearProbeTable_t * htable);

/* For awesome hash table */
int makeKey(int seed);
int makeSentinelKey(int seed);
bool add_bucketlist(bucketlist_t bucket, int key, volatile Packet_t *x); 
bool remove_bucketlist(bucketlist_t bucket, int key); 
bool contains_bucketlist(bucketlist_t bucket, int key);
node_t *getSentinel(bucketlist_t bucket, int idx);
bucketlist_t getBucket(awesomeTable_t *table, int idx); 
void initBucket(awesomeTable_t *table, int idx);
int getParent(awesomeTable_t *table, int idx); 
awesomeTable_t *createAwesomeTable(int logSize); 
void add_awesome(hashtable_t *htable, int key, volatile Packet_t *x);
bool remove_awesome(hashtable_t *htable, int key);
bool contains_awesome(hashtable_t *htable, int key);
void print_awesome(awesomeTable_t * htable);

void print_table(hashtable_t *htable, int type);
void free_htable(hashtable_t *htable, int type);
long countPkt(hashtable_t *htable, int type);

char *binrep (unsigned int val);
#endif /* HASHTABLE_H_ */
