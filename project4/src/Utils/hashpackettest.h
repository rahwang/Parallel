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
  volatile int *go;
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
			  ParallelPacketWorker_t *data,
			  HashPacketGenerator_t * source, 
			  int numWorkers,
			  volatile int *go,
			  HashList_t **queues,
			  long *fingerprints);

lockFreeCTable_t *initLockFreeC(int maxBucketSize, int initSize, 
				ParallelPacketWorker_t *data,
				HashPacketGenerator_t * source, 
				int numWorkers,
				volatile int *go,
				HashList_t **queues,
				long *fingerprints);

linearProbeTable_t *initLinearProbe(int maxBucketSize, int initSize,  
				    ParallelPacketWorker_t *data,
				    HashPacketGenerator_t * source, 
				    int numWorkers,
				    volatile int *go,
				    HashList_t **queues,
				    long *fingerprints);

awesomeTable_t *initAwesome(int maxBucketSize, int initSize, 
			    ParallelPacketWorker_t *data, 
			    HashPacketGenerator_t * source,
			    int numWorkers,
			    volatile int *go,
			    HashList_t **queues,
			    long *fingerprints);

hashtable_t *initTable(int maxBucketSize, int initSize, 
		       ParallelPacketWorker_t *data, 
		       HashPacketGenerator_t * source,
		       int numWorkers,
		       volatile int *go,
		       HashList_t **queues,
		       long *fingerprints,
		       int tableType);
void dispatch(void *args);
int countPkt(hashtable_t *htable, int type);

#endif /* HASHPACKETTEST_H_ */
