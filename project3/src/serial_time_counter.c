#include "time_counter.h"

int main(int argc, char *argv[]) 
{
  if (argc != 2) {
    fprintf(stderr, "Error: wrong number of arguments provided. Program expects 1 time argument.");
    exit(1);
  }

  unsigned int time = (unsigned int)atoi(argv[1]);

  serial_time(time);

  return 0;
}
