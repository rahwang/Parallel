#include "Utils/hashpackettest.h"


int main(int argc, char *argv[]) 
{
  if (argc != 8) {
    fprintf(stderr, "Error: wrong number of arguments provided. Program expects 7 arguments.\n");
    exit(1);
  }
  
  unsigned int numMilliseconds = (unsigned int)atoi(argv[1]);
  float fractionAdd = atof(argv[2]);
  float fractionRemove = atof(argv[3]);
  float hitRate = atof(argv[4]);
  int maxBucketSize = atoi(argv[5]);
  long mean = (long)atoi(argv[6]);
  int initSize = atoi(argv[7]);
  
  serialHashPacketTest(numMilliseconds, fractionAdd, fractionRemove, hitRate, maxBucketSize, mean, initSize);
}
