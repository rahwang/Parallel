#ifndef HASHPACKETTEST_H_
#define HASHPACKETTEST_H_

#include <time.h>
#include "stopwatch.h"
#include "hashgenerator.h"
#include "paddedprim.h"
#include "statistics.h"
#include <stdbool.h>
#include <pthread.h>
#include "hashpacketworker.h"
#include "hashtable.h"
#include "hashlist.h"

typedef struct dispatch_t{
  HashPacketGenerator_t *source;
  HashList_t **queues;
  volatile int n;
  volatile int count;
  PaddedPrimBool_NonVolatile_t * done;
} dispatch_t;

long serialHashPacketTest(int numMilliseconds,
			  float fractionAdd,
			  float fractionRemove,
			  float hitRate,
			  int maxBucketSize,
			  long mean,
			  int initSize);


long parallelHashPacketTest(int numMilliseconds,
			    float fractionAdd,
			    float fractionRemove,
			    float hitRate,
			    int maxBucketSize,
			    long mean,
			    int initSize,
			    int numWorkers,
			    int tableType);

lockedTable_t *initLocked(int maxBucketSize, int initSize, 
			  HashPacketGenerator_t *source, 
			  ParallelPacketWorker_t *data, 
			  int numWorkers,
			  PaddedPrimBool_NonVolatile_t * done,
			  HashList_t **queues,
			  long *fingerprints);

lockFreeCTable_t *initLockFreeC(int maxBucketSize, int initSize, 
			  HashPacketGenerator_t *source, 
			  ParallelPacketWorker_t *data, 
			  int numWorkers,
			  PaddedPrimBool_NonVolatile_t * done,
			  HashList_t **queues,
			  long *fingerprints);

linearProbeTable_t *initLinearProbe(int maxBucketSize, int initSize, 
			  HashPacketGenerator_t *source, 
			  ParallelPacketWorker_t *data, 
			  int numWorkers,
			  PaddedPrimBool_NonVolatile_t * done,
			  HashList_t **queues,
			  long *fingerprints);

awesomeTable_t *initAwesome(int maxBucketSize, int initSize, 
			  HashPacketGenerator_t *source, 
			  ParallelPacketWorker_t *data, 
			  int numWorkers,
			  PaddedPrimBool_NonVolatile_t * done,
			  HashList_t **queues,
			  long *fingerprints);

void dispatch(void *args);

#endif /* HASHPACKETTEST_H_ */
