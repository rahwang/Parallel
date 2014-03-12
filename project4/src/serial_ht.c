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

  int trials = 10;
  long res = 0;
  int i;
  for (i = 0; i < trials; i++) {
    res += serialHashPacketTest(numMilliseconds, fractionAdd, fractionRemove, hitRate, maxBucketSize, mean, initSize) / trials;
  }
  //printf("serial\tmilsecs\tadds\trems\thits\tmaxbuck\twork\tinit\n");
  printf("s\t%u\t%f\t%f\t%f\t%i\t%li\t%i\t%li\n", numMilliseconds, fractionAdd, fractionRemove, hitRate, maxBucketSize, mean, initSize, res);
}
