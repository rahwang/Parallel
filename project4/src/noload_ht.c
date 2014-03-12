#include "Utils/hashpackettest.h"


int main(int argc, char *argv[]) 
{
  int i;

  if (argc != 10) {
    fprintf(stderr, "Error: wrong number of arguments provided. Program expects 9 arguments.\n");
    exit(1);
  }

  unsigned int numMilliseconds = (unsigned int)atoi(argv[1]);
  float fractionAdd = atof(argv[2]);
  float fractionRemove = atof(argv[3]);
  float hitRate = atof(argv[4]);
  int maxBucketSize = atoi(argv[5]);
  long mean = (long)atoi(argv[6]);
  int initSize = atoi(argv[7]);
  int numWorkers = atoi(argv[8]);
  int tableType = atoi(argv[9]);

  int trials = 10;
  long res = 0;
  for (i = 0; i < trials; i++) {
    res += noloadHashPacketTest(numMilliseconds, fractionAdd, fractionRemove, hitRate, maxBucketSize, mean, initSize, numWorkers, tableType) / trials;
  }
  //printf("parallel\tmilsecs\tadds\trems\thits\tmaxbuck\twork\tinit\tn\ttype\n");
    printf("n\t%u\t%f\t%f\t%f\t%i\t%li\t%i\t%i\t%i\t%li\n", numMilliseconds, fractionAdd, fractionRemove, hitRate, maxBucketSize, mean, initSize, numWorkers, tableType, res);
}
