#include "work_counter.h"
  

int main(int argc, char *argv[]) 
{
  int i;

  if (argc != 3) {
    fprintf(stderr, "Error: wrong number of arguments provided. Program expects 2 arguments (work, expNum).");
    exit(1);
  }

  int work = atoi(argv[1]);
  int exp = atoi(argv[2]);

  int trials = 3;
  double res = 0;
  for (i = 0; i < trials; i++) {
    res += serial_work(work);
  }
  res /= trials;

  printf("%i\t%s\t%i\t%i\t%i\t%f\n", exp, "ser", work, 1, 0, res);

  return 0;
}
