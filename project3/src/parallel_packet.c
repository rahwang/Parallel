#include "packets.h"


int main(int argc, char *argv[])
{
  // get args
  if (argc != 7) 
    {
      fprintf(stderr, "Error: wrong number of arguments provided. Program expects 8 arguments.");
      exit(1);
    }
  
  unsigned int time =(unsigned int)atoi(argv[1]);
  int n = atoi(argv[2]);
  int W = atoi(argv[3]);
  int uni = atoi(argv[4]);
  int exp = atoi(argv[5]);
  int D = atoi(argv[6]);
  int type = atoi(argv[7]);
  int S = atoi(argv[8]);

  parallel_pack(time, n, W, uni, exp, D, type, S);
  
  return 0;
}
