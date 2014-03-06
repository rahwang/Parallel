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

void serialHashPacketTest(int numMilliseconds,
			  float fractionAdd,
			  float fractionRemove,
			  float hitRate,
			  int maxBucketSize,
			  long mean,
			  int initSize);


void parallelHashPacketTest(int numMilliseconds,
			    float fractionAdd,
			    float fractionRemove,
			    float hitRate,
			    int maxBucketSize,
			    long mean,
			    int initSize,
			    int numWorkers,
			    int tableType);



#endif /* HASHPACKETTEST_H_ */
