#ifndef HASHPACKETWORKER_H_
#define HASHPACKETWORKER_H_

#include "paddedprim.h"
#include "hashgenerator.h"
#include "hashtable.h"

typedef struct serialPacketWorker_t{
  PaddedPrimBool_NonVolatile_t * done;
  HashPacketGenerator_t * source;
  serialTable_t * table;
  long totalPackets;
  long residue;
  long fingerprint;
}serialPacketWorker_t;


void serialWorker(serialPacketWorker_t *data);


#endif /* HASHPACKETWORKER_H_ */
