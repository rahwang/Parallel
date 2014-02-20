#include "time_counter.h"


int main(int argc, char *argv[])
{
  // get args
  if (argc != 4) 
    {
      fprintf(stderr, "Error: wrong number of arguments provided. Program expects 3 arguments.");
      exit(1);
    }
  
  unsigned int time =(unsigned int)atoi(argv[1]);
  int n = atoi(argv[2]);
  int type = atoi(argv[3]);

  long counter = parallel_time(time, n, type);
  printf("%li\n", counter);
  return 0;
}


