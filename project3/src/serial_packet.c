#include "packets.h"


int main(int argc, char *argv[])
{
  int i;

  // get args
  if (argc != 6) 
    {
      fprintf(stderr, "Error: wrong number of arguments provided. Program expects 5 arguments.");
      exit(1);
    }
  
  unsigned int time =(unsigned int)atoi(argv[1]);
  int n = atoi(argv[2]);
  int W = atoi(argv[3]);
  int uni = atoi(argv[4]);
  short exp = atoi(argv[5]);

  int trials = 10;
  long counter = 0;
  for (i = 0; i < trials; i++) {
    counter += serial_pack(time, n, W, uni, exp);
  }

  counter /= trials;
  printf("%i\t%s\t%u\t%i\t%i\t%li\n", exp, "ser", time, n, W, counter);  
  return 0;
}
