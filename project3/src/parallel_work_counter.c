#include "work_counter.h"

int main(int argc, char *argv[])
{
  // get args
  if (argc != 4) 
    {
      fprintf(stderr, "Error: wrong number of arguments provided. Program expects 3 arguments.");
      exit(1);
    }
  
  int work = atoi(argv[1]);
  int n = atoi(argv[2]);
  int type = atoi(argv[3]);

  if (work % n != 0) {
    fprintf(stderr, "Error: work is not divisible by number of worker threads.");
    exit(1);
  }  

  parallel_work(work, n, type);
  
  return 0;
}
