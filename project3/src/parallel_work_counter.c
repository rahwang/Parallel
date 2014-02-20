#include "work_counter.h"

int main(int argc, char *argv[])
{
  int i;

  // get args
  if (argc != 5) 
    {
      fprintf(stderr, "Error: wrong number of arguments provided. Program expects 4 arguments (work, n, lock, expNum).");
      exit(1);
    }
  
  int work = atoi(argv[1]);
  int n = atoi(argv[2]);
  int type = atoi(argv[3]);
  int exp = atoi(argv[4]);

  if (work % n != 0) {
    fprintf(stderr, "Error: work is not divisible by number of worker threads.");
    exit(1);
  }  

  int trials = 3;
  double res = 0;
  for (i = 0; i < trials; i++) {
    res += parallel_work(work, n, type);
  }

  res /= trials;
  printf("%i\t%s\t%i\t%i\t%i\t%f\n", exp, "par", work, n, type, res);

  return 0;
}
