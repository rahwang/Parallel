#include "time_counter.h"


int main(int argc, char *argv[])
{
  int i;

  // get args
  if (argc != 5) 
    {
      fprintf(stderr, "Error: wrong number of arguments provided. Program expects 4 arguments (time, n, lock, expNum");
      exit(1);
    }
  
  unsigned int time =(unsigned int)atoi(argv[1]);
  int n = atoi(argv[2]);
  int type = atoi(argv[3]);
  int exp = atoi(argv[4]);

  int trials = 3;
  long counter = 0;
  for (i = 0; i < trials; i++) {
    counter += parallel_time(time, n, type, exp);
  }
  
  counter /= trials;
  printf("%i\t%s\t%u\t%i\t%i\t%li\n", exp, "par", time, n, type, counter);
  return 0;
}


