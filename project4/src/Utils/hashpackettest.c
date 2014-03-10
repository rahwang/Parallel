#include "hashpackettest.h"

void millToTimeSpec(struct timespec *ts, unsigned long ms)
{
  ts->tv_sec = ms / 1000;
  ts->tv_nsec = (ms % 1000) * 1000000;
}


double serialHashPacketTest(int numMilliseconds,
			  float fractionAdd,
			  float fractionRemove,
			  float hitRate,
			  int maxBucketSize,
			  long mean,
			  int initSize)
{
  
  StopWatch_t timer;
  
  // Create timer flag
  volatile int go = 1;
  
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
  
  SerialPacketWorker_t  workerData = {&go, source, table,0,0,0};
  
  rc = pthread_create(&workerThread, NULL, (void *) &serialWorker, (void *) &workerData);
  
  if (rc){
    fprintf(stderr,"ERROR; return code from pthread_create() for solo thread is %d\n", rc);
    exit(-1);
  }
  
  //printf("Sleeping!!\n");
  
  startTimer(&timer);
  
  nanosleep(&tim, NULL);
  
  go = 0;
  	
  rc = pthread_join(workerThread, NULL);
  if (rc){
    fprintf(stderr,"firewall error: return code for the threads using pthread_join() for solo thread  is %d\n", rc);
    exit(-1);
  }
  
  stopTimer(&timer);

  //print_serial(table);
  free_serial(table);
  
  double totalCount = (double)workerData.totalPackets;
  //printf("count: %ld \n", totalCount);
  //printf("time: %f\n",getElapsedTime(&timer));
  //printf("%f inc / ms\n", totalCount/getElapsedTime(&timer));

  return totalCount;
}


double parallelHashPacketTest(int numMilliseconds,
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
  volatile int go = 1;

  
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
  hashtable_t *htable = initTable(maxBucketSize, initSize, data, source, numWorkers, &go, queues, fingerprints, tableType);
  
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
  dispatchData.go = &go;
  
  // Start timing
  struct timespec tim;  
  millToTimeSpec(&tim,numMilliseconds);
  startTimer(&timer);
  
  // start your Dispatcher
  if ((rc = pthread_create(&dispatcher, NULL, (void *)&dispatch, &dispatchData))) {
    fprintf(stderr,"ERROR: return code from pthread_create() for dispatch thread is %d\n", rc);
    exit(-1);
  }
    
  // Sleep
  nanosleep(&tim , NULL);
 
  // assert signals to stop Dispatcher
  go = 0;  
  
  // call join on Dispatcher
  pthread_join(dispatcher, NULL);
  
  // call join for each Worker
  double totalCount = 0;
  for (i = 0; i < numWorkers; i++) {
    pthread_join(worker[i], NULL);
    totalCount += data[i].myCount;
  }
  
  // Stop timing
  stopTimer(&timer);

  
  
  free_htable(htable, tableType);
  // report the total number of packets processed and total time
  //printf("count: %f \n", totalCount);
  //printf("time: %f\n",getElapsedTime(&timer));
  //printf("%f inc / ms\n", totalCount/getElapsedTime(&timer));
  return totalCount;
}

hashtable_t *initTable(int maxBucketSize, int initSize, 
		       ParallelPacketWorker_t *data, 
		       HashPacketGenerator_t * source,
		       int numWorkers,
		       volatile int *go,
		       HashList_t **queues,
		       long *fingerprints,
		       int tableType)
{
  hashtable_t *htable = (hashtable_t *)malloc(sizeof(hashtable_t));

  switch(tableType) {
  case(LOCKED):
    htable->locked = initLocked(maxBucketSize, initSize, data, source, numWorkers, go, queues, fingerprints);
    break;
  case(LOCKFREEC):
    htable->lockFreeC = initLockFreeC(maxBucketSize, initSize, data, source, numWorkers, go, queues, fingerprints);
    break;/*
  case(LINEARPROBED):
    htable->linearProbe = initLinearProbe(maxBucketSize, initSize, data, source, numWorkers, go, queues, fingerprints);
    break;
  case(AWESOME):
    htable->awesome = initAwesome(maxBucketSize, initSize, data, source, numWorkers, go, queues, fingerprints);
    break;*/
  }

  return htable;
}

lockedTable_t *initLocked(int maxBucketSize, int initSize, 
			  ParallelPacketWorker_t *data, 
			  HashPacketGenerator_t * source,
			  int numWorkers,
			  volatile int* go,
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
    data[i].go = go;
    //data[i].totalPackets = 0;
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

lockFreeCTable_t *initLockFreeC(int maxBucketSize, int initSize, 
			  ParallelPacketWorker_t *data, 
			  HashPacketGenerator_t * source,
			  int numWorkers,
			  volatile int* go,
			  HashList_t **queues,
			  long *fingerprints) 
{
  int base = (initSize == 0) ? 2 : initSize*2;
  hashtable_t *htable = (hashtable_t *)malloc(sizeof(hashtable_t));
  htable->lockFreeC = createLockFreeCTable(base, maxBucketSize, numWorkers);
  HashPacket_t *pkt;

  int i;
  for (i = 0; i < initSize; i++) {
    pkt = getAddPacket(source);
    addNoCheck_lockFreeC(htable->lockFreeC, pkt->key, pkt->body);
  }
  for (i = 0; i < numWorkers; i++) {
    data[i].go = go;
    //data[i].totalPackets = 0;
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
  return htable->lockFreeC;
}
/*
linearProbeTable_t *initLinearProbe(int maxBucketSize, int initSize, 
				    HashPacketGenerator_t *source, 
				    ParallelPacketWorker_t *data, 
				    int numWorkers,
				    PaddedPrimBool_NonVolatile_t * go,
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
    data[i].go = go;
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
			    PaddedPrimBool_NonVolatile_t * go,
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
    data[i].go = go;
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
  int it = 0;
  while(*(data->go)) {
    for (i = 0; i < data->n; i++) {
      volatile HashPacket_t *pkt = getRandomPacket(data->source);
      data->count += enqueue((data->queues)[i], 12, pkt, data->count);
    }
    it++;
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
