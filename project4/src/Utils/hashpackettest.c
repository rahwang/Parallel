#include "hashpackettest.h"

void millToTimeSpec(struct timespec *ts, unsigned long ms)
{
  ts->tv_sec = ms / 1000;
  ts->tv_nsec = (ms % 1000) * 1000000;
}


long serialHashPacketTest(int numMilliseconds,
			  float fractionAdd,
			  float fractionRemove,
			  float hitRate,
			  int maxBucketSize,
			  long mean,
			  int initSize)
{
  
  StopWatch_t timer;
  
  PaddedPrimBool_NonVolatile_t done;
  done.value = false;
  
  // PaddedPrimBool_t memFence;
  // memFence.value = false;
  
  HashPacketGenerator_t * source = createHashPacketGenerator(fractionAdd,fractionRemove,hitRate,mean);
  serialTable_t * table = createSerialTable(1, maxBucketSize);
  
  for( int i = 0; i < initSize; i++ ) {
    HashPacket_t * pkt = getAddPacket(source);
    add_serial(table,mangleKey(pkt), pkt->body);
  }
  
  pthread_t workerThread;
  int rc;
  
  struct timespec tim;
  
  millToTimeSpec(&tim,numMilliseconds);
  
  SerialPacketWorker_t  workerData = {&done, source, table,0,0,0};
  
  rc = pthread_create(&workerThread, NULL, (void *) &serialWorker, (void *) &workerData);
  
  if (rc){
    fprintf(stderr,"ERROR; return code from pthread_create() for solo thread is %d\n", rc);
    exit(-1);
  }
  
  //printf("Sleeping!!\n");
  
  startTimer(&timer);
  
  nanosleep(&tim, NULL);
  
  done.value = true;
  
  //memFence.value = true;
	
  rc = pthread_join(workerThread, NULL);
  if (rc){
    fprintf(stderr,"firewall error: return code for the threads using pthread_join() for solo thread  is %d\n", rc);
    exit(-1);
  }
  
  stopTimer(&timer);

  free_serial(table);
  
  long totalCount = workerData.totalPackets;
  //printf("count: %ld \n", totalCount);
  //printf("time: %f\n",getElapsedTime(&timer));
  //printf("%f inc / ms\n", totalCount/getElapsedTime(&timer));

  return totalCount;
}


long parallelHashPacketTest(int numMilliseconds,
			    float fractionAdd,
			    float fractionRemove,
			    float hitRate,
			    int maxBucketSize,
			    long mean,
			    int initSize,
			    int numWorkers,
			    int tableType)
{
  StopWatch_t timer;
  int i, rc;
  PaddedPrimBool_NonVolatile_t done;
  done.value = false;
  
  // allocate and initialize queues + fingerprints
  HashList_t *queues[numWorkers];
  long fingerprints[numWorkers];
  for (i = 0; i < numWorkers; i++) {
    queues[i] = createHashList();
    fingerprints[i] = 0;
  }
  
  // Create packet source
  HashPacketGenerator_t * source = createHashPacketGenerator(fractionAdd,fractionRemove,hitRate,mean);
  
  // Initialize htable + arguments by type
  pthread_t worker[numWorkers];
  ParallelPacketWorker_t data[numWorkers];
  hashtable_t *htable = initTable(maxBucketSize, initSize, data, source, numWorkers, &done, queues, fingerprints, tableType);
  
  // Spawn Workers
  for (i = 0; i <numWorkers; i++) {
    if ((rc = pthread_create(worker+i, NULL, (void *) &parallelWorker, (void *) (data+i)))) {
      fprintf(stderr,"ERROR: return code from pthread_create() for thread is %d\n", rc);
      exit(-1);
    }
  }
  
  // Dispatcher 
  pthread_t dispatcher;
  dispatch_t dispatchData;
  dispatchData.source = source;
  dispatchData.queues = queues;
  dispatchData.n = numWorkers;
  dispatchData.count = 0;
  
  // Start timing
  struct timespec tim;  
  millToTimeSpec(&tim,numMilliseconds);
  startTimer(&timer);
  
  // start your Dispatcher
  if ((rc = pthread_create(&dispatcher, NULL, (void *)&dispatch, (void *)&dispatchData))) {
    fprintf(stderr,"ERROR: return code from pthread_create() for dispatch thread is %d\n", rc);
    exit(-1);
  }
    
  // Sleep
  nanosleep(&tim , NULL);
 
  // assert signals to stop Dispatcher
  done.value = true;  
  
  // call join on Dispatcher
  pthread_join(dispatcher, NULL);
  
  // call join for each Worker
  for (i = 0; i < numWorkers; i++) {
    pthread_join(worker[i], NULL);
  }
  
  // Stop timing
  stopTimer(&timer);
  
  free_htable(htable, tableType);
  // report the total number of packets processed and total time
  //printf("count: %ld \n", totalCount);
  //printf("time: %f\n",getElapsedTime(&timer));
  //printf("%f inc / ms\n", totalCount/getElapsedTime(&timer));
  return 0;
}

hashtable_t *initTable(int maxBucketSize, int initSize, 
		       ParallelPacketWorker_t *data, 
		       HashPacketGenerator_t * source,
		       int numWorkers,
		       PaddedPrimBool_NonVolatile_t * done,
		       HashList_t **queues,
		       long *fingerprints,
		       int tableType)
{
  hashtable_t *htable = (hashtable_t *)malloc(sizeof(hashtable_t));

  switch(tableType) {
  case(LOCKED):
    htable->locked = initLocked(maxBucketSize, initSize, data, source, numWorkers, done, queues, fingerprints);
    break;/*
  case(LOCKFREEC):
    htable->lockFreeC = initLockFreeC(maxBucketSize, initSize, data, source, numWorkers, done, queues, fingerprints);
    break;
  case(LINEARPROBED):
    htable->linearProbe = initLinearProbe(maxBucketSize, initSize, data, source, numWorkers, done, queues, fingerprints);
    break;
  case(AWESOME):
    htable->awesome = initAwesome(maxBucketSize, initSize, data, source, numWorkers, done, queues, fingerprints);
    break;*/
  }

  return htable;
}

lockedTable_t *initLocked(int maxBucketSize, int initSize, 
			  ParallelPacketWorker_t *data, 
			  HashPacketGenerator_t * source,
			  int numWorkers,
			  PaddedPrimBool_NonVolatile_t * done,
			  HashList_t **queues,
			  long *fingerprints) 
{
  int base = (initSize == 0) ? 2 : initSize*2;
  hashtable_t *htable = (hashtable_t *)malloc(sizeof(hashtable_t));
  htable->locked = createLockedTable(base, maxBucketSize, numWorkers);
  HashPacket_t *pkt;

  int i;
  for (i = 0; i < initSize; i++) {
    pkt = getAddPacket(source);
    addNoCheck_locked(htable->locked, pkt->key, pkt->body);
  }
  for (i = 0; i < numWorkers; i++) {
    data[i].done = done;
    data[i].totalPackets = 0;
    data[i].queues = queues;
    data[i].tid = i;
    data[i].n = numWorkers;
    data[i].fingerprints = fingerprints;
    data[i].addf = &add_locked;
    data[i].removef = &remove_locked;
    data[i].containsf = &contains_locked;
    data[i].myCount = 0;
    data[i].table = htable;
  }
  return htable->locked;
}
/*
lockFreeCTable_t *initLockFreeC(int maxBucketSize, int initSize, 
			  HashPacketGenerator_t *source, 
			  ParallelPacketWorker_t *data, 
			  int numWorkers,
			  PaddedPrimBool_NonVolatile_t * done,
			  HashList_t **queues,
			  long *fingerprints) 
{
  int base = (initSize == 0) ? 4 : initSize*2;
  lockFreeCTable_t *new = createLockFreeCTable(base, maxBucketSize);
  HashPacket_t *pkt;

  int i;
  for (i = 0; i < initSize; i++) {
    pkt = getAddPacket(source);
    addNoCheck_lockFreeC(new, pkt->key, pkt->body);
  }
  for (i = 0; i < numWorkers; i++) {
    data[i].done = done;
    data[i].totalPackets = 0;
    data[i].queues = queues;
    data[i].tid = i;
    data[i].n = numWorkers;
    data[i].fingerprints = fingerprints;
    data[i].addf = &add_lockFreeC;
    data[i].removef = &remove_lockFreeC;
    data[i].containsf = &contains_lockFreeC;
    data[i].myCount = 0;
    data[i].table = htable;
  }

  return new;
}

linearProbeTable_t *initLinearProbe(int maxBucketSize, int initSize, 
				    HashPacketGenerator_t *source, 
				    ParallelPacketWorker_t *data, 
				    int numWorkers,
				    PaddedPrimBool_NonVolatile_t * done,
				    HashList_t **queues,
				    long *fingerprints) 

{
  int base = (initSize == 0) ? 4 : initSize*2;
  linearProbeTable_t *new = createLinearProbeTable(base, maxBucketSize);
  HashPacket_t *pkt;

  int i;
  for (i = 0; i < initSize; i++) {
    pkt = getAddPacket(source);
    addNoCheck_linearProbe(new, pkt->key, pkt->body);
  }
  for (i = 0; i < numWorkers; i++) {
    data[i].done = done;
    data[i].totalPackets = 0;
    data[i].queues = queues;
    data[i].tid = i;
    data[i].n = numWorkers;
    data[i].fingerprints = fingerprints;
    data[i].addf = &add_linearProbe;
    data[i].removef = &remove_linearProbe;
    data[i].containsf = &contains_linearProbe;
    data[i].myCount = 0;
    data[i].table = htable;
  }

  return new;
}

awesomeTable_t *initAwesome(int maxBucketSize, int initSize, 
			    HashPacketGenerator_t *source, 
			    ParallelPacketWorker_t *data, 
			    int numWorkers,
			    PaddedPrimBool_NonVolatile_t * done,
			    HashList_t **queues,
			    long *fingerprints) 
 
{
  int base = (initSize == 0) ? 4 : initSize*2;
  awesomeTable_t *new = createAwesomeTable(base, maxBucketSize);
  HashPacket_t *pkt;

  int i;
  for (i = 0; i < initSize; i++) {
    pkt = getAddPacket(source);
    addNoCheck_awesome(new, pkt->key, pkt->body);
  }
  for (i = 0; i < numWorkers; i++) {
    data[i].done = done;
    data[i].totalPackets = 0;
    data[i].queues = queues;
    data[i].tid = i;
    data[i].n = numWorkers;
    data[i].fingerprints = fingerprints;
    data[i].addf = &add_awesome;
    data[i].removef = &remove_awesome;
    data[i].containsf = &contains_awesome;
    data[i].myCount = 0;
    data[i].table = htable;
  }
  
  return new;
}
*/
void dispatch(void *args) 
{
  dispatch_t *data = (dispatch_t *)args;
  
  int i;
  while(!data->done->value) {
    for (i = 0; i < data->n; i++) {
      volatile HashPacket_t *pkt = getRandomPacket(data->source);
      data->count += enqueue((data->queues)[i], 12, pkt, data->count);
    }
  }
}


int countPkt(hashtable_t *htable, int type) {
  int i, size;
  int sum = 0;
  SerialList_t *bucket;
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
      bucket = htable->locked->table[i];
      sum += list_len(bucket);
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
