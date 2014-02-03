#include "funcs.h"

#define DEFAULT_NUMBER_OF_ARGS 6

// you can do it Rachel!!


int main(int argc, char * argv[]) {
  
  if(argc >= DEFAULT_NUMBER_OF_ARGS) {
    const int numPackets = atoi(argv[1]);
    const int numSources = atoi(argv[2]);
    const long mean = atol(argv[3]);
    const int uniformFlag = atoi(argv[4]);
    const short experimentNumber = (short)atoi(argv[5]);
    const int queueDepth = atoi(argv[6]);

    parallel_firewall(numPackets,numSources,mean,uniformFlag,experimentNumber, queueDepth);
  }
  return 0;
}
