#include "packets.h"


int main(int argc, char *argv[])
{
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

  long *fingerprint = (long *)malloc(sizeof(long)*n);

  serial_pack(time, n, W, uni, exp, fingerprint);
  
  return 0;
}
