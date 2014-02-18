#include "work_counter.h"
  

int main(int argc, char *argv[]) 
{
  if (argc != 2) {
    fprintf(stderr, "Error: wrong number of arguments provided. Program expects 1 work argument.");
    exit(1);
  }

  int work = atoi(argv[1]);

  serial_work(work);

  return 0;
}
