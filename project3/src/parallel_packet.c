#include "packets.h"
#include "joshutil.h"
#include <signal.h>


int run(int argc, char*argv[]){

  int i;
  // get args
  if (argc != 9) 
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
  
  int trials = 1;
  long counter = 0;
  for (i = 0; i < trials; i++) {
    counter += parallel_pack(time, n, W, uni, exp, D, type, S);
  }
  
  counter /= trials;
  printf("%i\t%s\t%u\t%i\t%i\t%i\t%i%li\n", exp, "par", time, n, W, type, S, counter);
  return 0;

}


int main(int argc, char *argv[])
{
  CATCH_ALL(run(argc, argv));
  return 0;

}
