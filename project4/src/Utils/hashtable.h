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
  int logSize;
  int mask;
  int maxBucketSize;
  int size;
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
  volatile SerialList_t ** table;
  volatile int numLocks;
  pthread_mutex_t *locks;
} lockFreeCTable_t;

typedef struct linearProbeTable_t {
  int logSize;
  int mask;
  int maxBucketSize;
  int size;
} linearProbeTable_t;

typedef struct awesomeTable_t {
  int logSize;
  int mask;
  int maxBucketSize;
  int size;
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

/* For lock-free-contains hash table 
lockFreeCTable_t * createLockFreeCTable(int logSize, int maxBucketSize);
void resizeIfNecessary_lockFreeC(lockFreeCTable_t * htable,int key);
void addNoCheck_lockFreeC(lockFreeCTable_t * htable,int key, volatile Packet_t * x);
void add_lockFreeC(hashtable_t * htable,int key, volatile Packet_t * x);
bool remove_lockFreeC(hashtable_t * htable,int key);
bool contains_lockFreeC(hashtable_t * htable,int key);
void resize_lockFreeC(lockFreeCTable_t * htable);
void print_lockFreeC(lockFreeCTable_t * htable);
*/
/* For serial hash table
linearProbeTable_t * createLinearProbeTable(int logSize, int maxBucketSize);
void resizeIfNecessary_linearProbe(linearProbeTable_t * htable,int key);
void addNoCheck_linearProbe(linearProbeTable_t * htable,int key, volatile Packet_t * x);
void add_linearProbe(hashtable_t * htable,int key, volatile Packet_t * x);
bool remove_linearProbe(hashtable_t * htable,int key);
bool contains_linearProbe(hashtable_t * htable,int key);
void resize_linearProbe(linearProbeTable_t * htable);
void print_linearProbe(linearProbeTable_t * htable);
*/
/* For serial hash table
awesomeTable_t * createAwesomeTable(int logSize, int maxBucketSize);
void resizeIfNecessary_awesome(awesomeTable_t * htable,int key);
void addNoCheck_awesome(awesomeTable_t * htable,int key, volatile Packet_t * x);
void add_awesome(hashtable_t * htable,int key, volatile Packet_t * x);
bool remove_awesome(hashtable_t * htable,int key);
bool contains_awesome(hashtable_t * htable,int key);
void resize_awesome(awesomeTable_t * htable);
void print_awesome(awesomeTable_t * htable);
*/

void free_htable(hashtable_t *htable, int type);

#endif /* HASHTABLE_H_ */
