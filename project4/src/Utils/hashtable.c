#include "hashtable.h"
#include <stdlib.h>
#include <stdio.h>

#define POLICY .75

/* SERIAL TABLE FUNCTIONS */

serialTable_t * createSerialTable(int logSize, int maxBucketSize)
{
  serialTable_t * htable = (serialTable_t *)malloc(sizeof(serialTable_t));
  htable->logSize = logSize;
  htable->maxBucketSize = maxBucketSize;
  htable->mask = (1 << logSize) - 1;
  int tableSize = (1 << logSize);
  htable->size = tableSize;
  htable->table = (SerialList_t **)malloc(sizeof(SerialList_t*)* tableSize);
  for(int i =0; i < tableSize; i++)
    htable->table[i] = NULL;
  
  return htable;
}

void resizeIfNecessary_serial(serialTable_t * htable, int key){
  while( htable->table[key & htable->mask] != NULL
	 && htable->table[key & htable->mask]->size >= htable->maxBucketSize)
    resize_serial(htable);
}

void addNoCheck_serial(serialTable_t * htable,int key, volatile Packet_t * x){
    int index = key & htable->mask;
    if( htable->table[index] == NULL )
    	htable->table[index] = createSerialListWithItem(key,x);
    else
    	addNoCheck_list(htable->table[index],key,x);
}

void add_serial(serialTable_t * htable,int key, volatile Packet_t * x){
  resizeIfNecessary_serial(htable,key);
  addNoCheck_serial(htable,key,x);
}

bool remove_serial(serialTable_t * htable,int key){
  resizeIfNecessary_serial(htable,key);
  if( htable->table[key & htable->mask] != NULL )
    return remove_list(htable->table[key & htable->mask],key);
  else
    return false;
}

bool contains_serial(serialTable_t * htable,int key){
    int myMask = htable->size - 1;
    if( htable->table[key & myMask] != NULL )
      return contains_list(htable->table[key & myMask],key);
    else
      return false;
}

void resize_serial(serialTable_t * htable){
  int newTableSize = htable->size * 2;
  SerialList_t ** newTable = (SerialList_t **)malloc(sizeof(SerialList_t*)* newTableSize);
  for(int i=0; i < newTableSize; i++) {
    newTable[i] = NULL;
  }

  for( int i = 0; i < htable->size; i++ ) {
    if( htable->table[i] != NULL){
      Item_t * curr = htable->table[i]->head;
      while( curr != NULL) {
	Item_t * nextItem = curr->next;
	if(newTable[curr->key & ((2*htable->mask)+1)] == NULL){
	  newTable[curr->key & ((2*htable->mask)+1)] = createSerialList();
	  curr->next = NULL;
	  newTable[curr->key & ((2*htable->mask)+1)]->head = curr;
	}else {
	  curr->next = newTable[curr->key & ((2*htable->mask)+1)]->head;
	  newTable[curr->key & ((2*htable->mask)+1)]->head = curr;
	}
	curr=nextItem;
      }
    }
  }
  SerialList_t ** temp = htable->table;
  htable->logSize++;
  htable->size = htable->size * 2;
  htable->mask = (1 << htable->logSize) - 1;
  htable->table = newTable;
  free(temp);
}


void free_serial(serialTable_t *table) {
  int i;
  for (i = 0; i < table->size; i++) {
    free(table->table[i]);
  }
  free(table->table);
  free(table);
}


void print_serial(serialTable_t * htable){
  for( int i = 0; i <= htable->mask; i++ ) {
    printf(".... %d ....",i);
    if(htable->table[i] != NULL)
      print_list(htable->table[i]);
  }
}




/* LOCKED TABLE FUNCTIONS */

lockedTable_t * createLockedTable(int logSize, int maxBucketSize, int n)
{
  int i;
  lockedTable_t * htable = (lockedTable_t *)malloc(sizeof(lockedTable_t));
  htable->entries = (int *)malloc(sizeof(int));
  *(htable->entries) = 0;
  htable->logSize = logSize;
  htable->maxBucketSize = maxBucketSize;
  htable->mask = (1 << logSize) - 1;
  int tableSize = (1 << logSize);
  htable->size = tableSize;
  htable->table = (SerialList_t **)malloc(sizeof(SerialList_t*)* tableSize);
  for(i =0; i < tableSize; i++) {
    htable->table[i] = NULL;
  }
  htable->numLocks = n;
  pthread_rwlock_t *rw_locks = (pthread_rwlock_t *)malloc(sizeof(pthread_rwlock_t)*n);
  for (i = 0; i < n; i++) {
    pthread_rwlock_init(rw_locks+i, NULL);
  }
  htable->rw_locks = rw_locks;
  return htable;
}


void resizeIfNecessary_locked(lockedTable_t * table, int key){	
  int i;
  int idx = key & table->mask;
  int oldsize = table->logSize;

  //if (*(table->entries) > (table->size)*POLICY) {
  if (table->table[idx] != NULL
      && table->table[idx]->size >= table->maxBucketSize) {
    for (i = 0; i < (table->numLocks); i++) {
      pthread_rwlock_wrlock((table->rw_locks)+i);
      //printf("Lock %i\n", i);
    }
    if (oldsize == table->logSize) {
	resize_locked(table);
    }
    for (i = 0; i < (table->numLocks); i++) {
      pthread_rwlock_unlock((table->rw_locks)+i);
      //printf("Unlock %i\n", i);
    }
  }
}


void addNoCheck_locked(lockedTable_t * table, int key, volatile Packet_t * x)
{
  pthread_rwlock_t *locks = table->rw_locks;
  int oldSize = table->logSize;
  int idx = key & table->mask;
  int lidx = idx % table->numLocks;

  pthread_rwlock_wrlock(locks+lidx);
  //printf("Lock %i\n", lidx);
  if (oldSize != table->logSize) {
    pthread_rwlock_unlock(locks+lidx);
    addNoCheck_locked(table, key, x);
    return;
  }
  if(table->table[idx] == NULL) {
    table->table[idx] = createSerialListWithItem(key, x);
  }
  else {
    //printf("Adding to list\n"); 
    addNoCheck_list(table->table[idx],key,x);
  }
  pthread_rwlock_unlock(locks+lidx);
  //printf("unlock %i\n", lidx);
}


void add_locked(hashtable_t * htable, int key, volatile Packet_t * x)
{
  lockedTable_t *table = htable->locked;
  resizeIfNecessary_locked(table, key);
  addNoCheck_locked(table, key, x);
}


bool remove_locked(hashtable_t * htable,int key)
{
  lockedTable_t *table = htable->locked;
  pthread_rwlock_t *locks = table->rw_locks;
  //resizeIfNecessary_locked(table, key);

  bool res;
  int oldSize = table->logSize;
  int idx = key & table->mask;
  int lidx = idx % table->numLocks;

  pthread_rwlock_wrlock(locks+lidx);
  //printf("Lock %i\n", lidx);
  if (oldSize != table->logSize) {
    pthread_rwlock_unlock(locks+lidx);
    return remove_locked(htable, key);
  }
  if( table->table[idx] != NULL ) {
    res = remove_list(table->table[idx],key);
  }
  else {
    res = false;
  }
  pthread_rwlock_unlock(locks+lidx);
  //printf("Unlock %i\n", lidx);
  return res;
}


bool contains_locked(hashtable_t * htable, int key) 
{
  lockedTable_t *table = htable->locked;
  pthread_rwlock_t *locks = table->rw_locks;
  bool res;
  int oldSize = table->logSize;
  int idx = key & table->mask;
  int lidx = idx % table->numLocks;
  
  pthread_rwlock_rdlock(locks+lidx);
  //printf("Lock %i\n", lidx);
  if (oldSize != table->logSize) {
    pthread_rwlock_unlock(locks+lidx);
    return contains_locked(htable, key);
  }
  if(table->table[idx] != NULL ) {
    res = contains_list(table->table[idx],key);
  }
  else {
    res = false;
  }
  pthread_rwlock_unlock(locks+lidx);
  //printf("Unlock %i\n", lidx);
  return res;
}

void resize_locked(lockedTable_t * table)
{
  int i;
  int newTableSize = table->size * 2;
  SerialList_t ** newTable = (SerialList_t **)malloc(sizeof(SerialList_t*)* newTableSize);
  for(i = 0; i < newTableSize; i++) {
    newTable[i] = NULL;
  }
  
  for(i = 0; i < table->size; i++ ) 
    {
      if( table->table[i] != NULL)
	{
	  Item_t * curr = table->table[i]->head;
	  while( curr != NULL) 
	    {
	      Item_t * nextItem = curr->next;
	      if(newTable[curr->key & ((2*table->mask)+1)] == NULL) {
		newTable[curr->key & ((2*table->mask)+1)] = createSerialList();
		curr->next = NULL;
		newTable[curr->key & ((2*table->mask)+1)]->head = curr;
	      }
	      else {
		curr->next = newTable[curr->key & ((2*table->mask)+1)]->head;
		newTable[curr->key & ((2*table->mask)+1)]->head = curr;
	      }
	      curr=nextItem;
	    }
	}
    }
  SerialList_t ** temp = table->table;
  table->logSize++;
  table->size = table->size * 2;
  table->mask = (1 << table->logSize) - 1;
  table->table = newTable;
  free(temp);
}


void print_locked(lockedTable_t * htable)
{
  printf("\n~~~ Locked Table ~~~\nSize = %i\nLogSize = %i\n\n", htable->size, htable->logSize);
  for( int i = 0; i <= htable->mask; i++ ) {
    printf("BUCKET %d ...",i);
    if(htable->table[i] != NULL) {
      print_list(htable->table[i]);
    }
    printf("\n");
  }
  printf("~~~ End Table ~~~\n\n");
}


void free_htable(hashtable_t *htable, int type) {
  int i;
  lockedTable_t *locked = NULL;
  //lockFreeCTable_t *lockFreeC;
  //linearProbeTable_t *linearProbe;
  //awesomeTable_t *awesome;

  switch(type) {
  case(LOCKED):
    locked = htable->locked;
    for (i = 0; i < locked->size; i++) {
      free(locked->table[i]);
    }
    for (i = 0; i < locked->numLocks; i++) {
      pthread_rwlock_destroy(locked->rw_locks+i);
    }
    free(locked->table);
    free(locked);
    break;
  }
  free(htable);
}




/* LOCK FREE C TABLE FUNCTIONS */

lockFreeCTable_t * createLockFreeCTable(int logSize, int maxBucketSize, int n)
{
  int i;
  lockFreeCTable_t * htable = (lockFreeCTable_t *)malloc(sizeof(lockFreeCTable_t));
  htable->entries = (int *)malloc(sizeof(int));
  *(htable->entries) = 0;
  htable->logSize = logSize;
  htable->maxBucketSize = maxBucketSize;
  htable->mask = (1 << logSize) - 1;
  int tableSize = (1 << logSize);
  htable->size = tableSize;
  htable->table = (SerialList_t **)malloc(sizeof(SerialList_t*)* tableSize);
  for(i =0; i < tableSize; i++) {
    htable->table[i] = NULL;
  }
  htable->numLocks = n;
  pthread_mutex_t *locks = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t)*n);
  for (i = 0; i < n; i++) {
    pthread_mutex_init(locks+i, NULL);
  }
  htable->locks = locks;
  return htable;
}


void resizeIfNecessary_lockFreeC(lockFreeCTable_t * table, volatile int key){	
  int i;
  int idx = key & table->mask;
  int oldsize = table->logSize;

  //if (*(table->entries) > (table->size)*POLICY) {
  if (table->table[idx] != NULL
      && table->table[idx]->size >= table->maxBucketSize) {
    for (i = 0; i < (table->numLocks); i++) {
      pthread_mutex_lock((table->locks)+i);
      //printf("Lock %i\n", i);
    }
    if (oldsize == table->logSize) {
	resize_lockFreeC(table);
    }
    for (i = 0; i < (table->numLocks); i++) {
      pthread_mutex_unlock((table->locks)+i);
      //printf("Unlock %i\n", i);
    }
  }
}


void addNoCheck_lockFreeC(lockFreeCTable_t * table, int key, volatile Packet_t * x)
{
  pthread_mutex_t *locks = table->locks;
  int oldSize = table->logSize;
  int idx = key & table->mask;
  int lidx = idx % table->numLocks;

  pthread_mutex_lock(locks+lidx);
  //printf("Lock %i\n", lidx);
  if (oldSize != table->logSize) {
    pthread_mutex_unlock(locks+lidx);
    addNoCheck_lockFreeC(table, key, x);
    return;
  }
  if(table->table[idx] == NULL) {
    table->table[idx] = createSerialListWithItem(key, x);
  }
  else {
    //printf("Adding to list\n"); 
    addNoCheck_list(table->table[idx],key,x);
  }
  pthread_mutex_unlock(locks+lidx);
  //printf("unlock %i\n", lidx);
}


void add_lockFreeC(hashtable_t * htable, int key, volatile Packet_t * x)
{
  lockFreeCTable_t *table = htable->lockFreeC;
  resizeIfNecessary_lockFreeC(table, key);
  addNoCheck_lockFreeC(table, key, x);
}


bool remove_lockFreeC(hashtable_t * htable,int key)
{
  lockFreeCTable_t *table = htable->lockFreeC;
  pthread_mutex_t *locks = table->locks;
  //resizeIfNecessary_lockFreeC(table, key);

  bool res;
  int oldSize = table->logSize;
  int idx = key & table->mask;
  int lidx = idx % table->numLocks;

  pthread_mutex_lock(locks+lidx);
  //printf("Lock %i\n", lidx);
  if (oldSize != table->logSize) {
    pthread_mutex_unlock(locks+lidx);
    return remove_lockFreeC(htable, key);
  }
  if( table->table[idx] != NULL ) {
    res = remove_list(table->table[idx],key);
  }
  else {
    res = false;
  }
  pthread_mutex_unlock(locks+lidx);
  //printf("Unlock %i\n", lidx);
  return res;
}


bool contains_lockFreeC(hashtable_t * htable, int key) 
{
  lockFreeCTable_t *table = htable->lockFreeC;
  bool res;
  int oldSize = table->logSize;
  int idx = key & table->mask;
  
  if(table->table[idx] != NULL ) {
    res = contains_list(table->table[idx],key);
  }
  else {
    res = false;
  }
  if (table->logSize != oldSize) {
    return contains_lockFreeC(htable, key);
  }
  return res;
}

void resize_lockFreeC(lockFreeCTable_t * table)
{
  int i;
  int newTableSize = table->size * 2;
  SerialList_t ** newTable = (SerialList_t **)malloc(sizeof(SerialList_t*)* newTableSize);
  for(i = 0; i < newTableSize; i++) {
    newTable[i] = NULL;
  }
  
  for(i = 0; i < table->size; i++ ) 
    {
      if( table->table[i] != NULL)
	{
	  Item_t * curr = table->table[i]->head;
	  while( curr != NULL) 
	    {
	      Item_t * nextItem = curr->next;
	      if(newTable[curr->key & ((2*table->mask)+1)] == NULL) {
		newTable[curr->key & ((2*table->mask)+1)] = createSerialList();
		curr->next = NULL;
		newTable[curr->key & ((2*table->mask)+1)]->head = curr;
	      }
	      else {
		curr->next = newTable[curr->key & ((2*table->mask)+1)]->head;
		newTable[curr->key & ((2*table->mask)+1)]->head = curr;
	      }
	      curr=nextItem;
	    }
	}
    }
  //SerialList_t ** temp = table->table;
  table->logSize++;
  table->size = table->size * 2;
  table->mask = (1 << table->logSize) - 1;
  table->table = newTable;
  //free(temp);
}

void print_lockFreeC(lockFreeCTable_t * htable)
{
  printf("\n~~~ Locked Table ~~~\nSize = %i\nLogSize = %i\n\n", htable->size, htable->logSize);
  for( int i = 0; i <= htable->mask; i++ ) {
    printf("BUCKET %d ...",i);
    if(htable->table[i] != NULL) {
      print_list(htable->table[i]);
    }
    printf("\n");
  }
  printf("~~~ End Table ~~~\n\n");
}




/* LINEAR PROBE TABLE FUNCTIONS */

linearProbeTable_t * createLinearProbeTable(int logSize, int maxStep, int n)
{
  int i;
  linearProbeTable_t * htable = (linearProbeTable_t *)malloc(sizeof(linearProbeTable_t));
  htable->entries = (int *)malloc(sizeof(int));
  *(htable->entries) = 0;
  htable->logSize = logSize;
  htable->maxStep = maxStep;
  htable->mask = (1 << logSize) - 1;
  int tableSize = (1 << logSize);
  htable->size = tableSize;
  htable->table = (Pack_t *)malloc(sizeof(Pack_t)* tableSize);
  for(i =0; i < tableSize; i++) {
    htable->table[i].value = NULL;
  }
  htable->numLocks = n;
  pthread_mutex_t *locks = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t)*n);
  for (i = 0; i < n; i++) {
    pthread_mutex_init(locks+i, NULL);
  }
  htable->locks = locks;
  htable->owned = 0;
  return htable;
}


void resizeIfNecessary_linearProbe(linearProbeTable_t * table, volatile int key){	
  int i;
  int oldsize = table->logSize;

  if (__sync_lock_test_and_set(&table->owned, 1)) {
    return;
  }
  else {
    for (i = 0; i < (table->numLocks); i++) {
      pthread_mutex_lock((table->locks)+i);
      //printf("Lock %i\n", i);
    }
    if (oldsize == table->logSize) {
      resize_linearProbe(table);
    }
    for (i = 0; i < (table->numLocks); i++) {
      pthread_mutex_unlock((table->locks)+i);
      //printf("Unlock %i\n", i);
    }
    table->owned = 0;
  }
}


void add_linearProbe(hashtable_t * htable, int key, volatile Packet_t * x)
{
  linearProbeTable_t *table = htable->linearProbe;
  pthread_mutex_t *locks = table->locks;
  int oldSize = table->logSize;
  int idx = key & table->mask;
  int numLocks = table->numLocks;
  int lidx;
  int steps = 0;
  //printf("begin call\n");

  while (steps < table->maxStep) {
    lidx = ((idx+steps) % numLocks);
    pthread_mutex_lock(locks+lidx);
    //printf("lidx %d numlock %i\n", lidx, numLocks);
    if ((table->owned) || (oldSize != table->logSize)) {
      pthread_mutex_unlock(locks+lidx);
      return add_linearProbe(htable, key, x);
    }
    if (table->table[(idx+steps) % table->size].value == NULL) {
      table->table[(idx+steps) % table->size].value = x;
      table->table[(idx+steps) % table->size].key = key;
      pthread_mutex_unlock(locks+lidx);
      //printf("%i Successful add to %i\n", lidx, (idx+steps) % table->size);
      //printf("Pkts = %li, size = %i\n", countPkt(htable, 3), table->size);
      return;
    }
    pthread_mutex_unlock(locks+lidx);
    steps++;
  }
  resizeIfNecessary_linearProbe(table, key);
  add_linearProbe(htable, key, x);
}


bool remove_linearProbe(hashtable_t * htable, int key)
{
  linearProbeTable_t *table = htable->linearProbe;
  pthread_mutex_t *locks = table->locks;
  //resizeIfNecessary_linearProbe(table, key);

  int oldSize = table->logSize;
  int idx = key & table->mask;
  int numLocks = table->numLocks;
  int lidx;
  int steps = 0;

  while (steps < table->maxStep) {
    lidx = ((idx + steps) % numLocks);
    pthread_mutex_lock(locks+lidx);
    if ((table->owned) || (oldSize != table->logSize)) {
      pthread_mutex_unlock(locks+lidx);
      return remove_linearProbe(htable, key);
    }
    if ((table->table[(idx+steps) % table->size].value) &&
	table->table[(idx+steps) % table->size].key == key) {
      table->table[(idx+steps) % table->size].key = 0;
      free((Packet_t *)table->table[(idx+steps) % table->size].value);
      table->table[(idx+steps) % table->size].value = NULL;
      pthread_mutex_unlock(locks+lidx);
      return true;
    }
    pthread_mutex_unlock(locks+lidx);
    steps++;
  }
  return false;
}


bool contains_linearProbe(hashtable_t * htable, int key) 
{
  linearProbeTable_t *table = htable->linearProbe;
  pthread_mutex_t *locks = table->locks;
  //resizeIfNecessary_linearProbe(table, key);

  int oldSize = table->logSize;
  int idx = key & table->mask;
  int numLocks = table->numLocks;
  int lidx;
  int steps = 0;

  while (steps < table->maxStep) {
    lidx = ((idx + steps) % numLocks);
    if ((table->owned) || (oldSize != table->logSize)) {
      return contains_linearProbe(htable, key);
    }
    else {
      pthread_mutex_lock(locks+lidx);
    }
    if ((table->table[(idx+steps) % table->size].value) &&
	table->table[(idx+steps) % table->size].key == key) {
      pthread_mutex_unlock(locks+lidx);
      return true;
    }
    pthread_mutex_unlock(locks+lidx);
    steps++;
  }
  return false;
}


void resize_linearProbe(linearProbeTable_t * table)
{
  int i, steps, idx;
  table->logSize++;
  table->mask = (1 << table->logSize) - 1;
  int newTableSize = (1 << table->logSize);
  Pack_t *newTable = (Pack_t *)malloc(sizeof(Pack_t)*newTableSize);
  volatile Packet_t *pkt;
  int key;
  for(i = 0; i < newTableSize; i++) {
    newTable[i].value = NULL;
  }

  //hashtable_t *tmp = (hashtable_t *)malloc(sizeof(hashtable_t));
  //tmp->linearProbe = table;
  //printf("IN Pkts = %li\n", countPkt(tmp, 3));
  for(i = 0; i < table->size; i++ ) {
    if ((pkt = table->table[i].value)) {
      key = table->table[i].key;
      steps = 0;
      while(steps < table->maxStep) {
	idx = ((key & table->mask)+steps) % newTableSize;
	//printf("idx = %i\n", idx);
	if (newTable[idx].value == NULL) {
	  newTable[idx].value = pkt;
	  newTable[idx].key = key;
	  steps = 1000;
	  break;
	}
	steps++;
      }
      if (steps != 1000) {
	//printf("resize = %i\n", table->logSize);
	return resize_linearProbe(table);
      }
    }
  }


  Pack_t *temp = table->table;
  table->size = newTableSize;
  table->table = newTable;
  //printf("OUT Pkts = %li\n", countPkt(tmp, 3));
  free(temp);
}


void print_linearProbe(linearProbeTable_t * htable)
{
  int i;
  printf("\n~~~ Locked Table ~~~\nSize = %i\nLogSize = %i\n\n", htable->size, htable->logSize);
  for(i = 0; i < htable->size; i++ ) {
    printf("BUCKET %d ...",i);
    if(htable->table[i].value != NULL) {
      printf("KEY:%i\n", htable->table[i].key);
    }
    printf("\n");
  }
  printf("~~~ End Table ~~~\n\n");
}

long countPkt(hashtable_t *htable, int type) {
  int i, size;
  long sum = 0;
  SerialList_t *bucket = NULL;

  switch(type) {
  case (LOCKED):
    size = htable->locked->size;
    for (i = 0; i < size; i++) {
      bucket = htable->locked->table[i];
      sum += list_len(bucket);
    }
    break;
  case (LOCKFREEC):
    size = htable->lockFreeC->size;
    for (i = 0; i < size; i++) {
      bucket = htable->locked->table[i];
      sum += list_len(bucket);
    }
    break;
  case (LINEARPROBED):
    size = htable->linearProbe->size;
    for (i = 0; i < size; i++) {
      sum += (htable->linearProbe->table[i].value != NULL);
    }
    break;
  case (AWESOME):
    size = htable->awesome->size;
    for (i = 0; i < size; i++) {
      bucket = htable->locked->table[i];
      sum += list_len(bucket);
    }
    break;
  }
  return sum;
}
