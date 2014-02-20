#include "time_counter.h"

int main(int argc, char *argv[]) 
{
  int i;

  if (argc != 3) {
    fprintf(stderr, "Error: wrong number of arguments provided. Program expects 2 arguments (time, expNum).\n");
    exit(1);
  }

  unsigned int time = (unsigned int)atoi(argv[1]);
  int exp = atoi(argv[2]);

  int trials = 3;
  long counter = 0;
  for (i = 0; i < trials; i++) {
    counter += serial_time(time);
  }
  counter /= trials;

  printf("%i\t%s\t%u\t%i\t%i\t%li\n", exp, "ser", time, 1, 0, counter);

  return 0;
}
