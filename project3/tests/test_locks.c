#include "../src/locks.h"
#include "../src/time_counter.h"
#include "../src/work_counter.h"
#include "../src/packets.h"

#define GREEN "\033[0;32m"
#define NORM "\033[0m"
#define RED "\033[0;31m"

#define DONE 1000000

#define TAS 1
#define BACK 2
#define MUTEX 3
#define ALOCK 4
#define CLH 5

int trials;

/* Format test results into pretty print statement */
void res(int pass, char *test, char *arg) 
{
  if (pass) {
    printf("| %15s  %30s: %s \t\tPASSED %s|\n", test, arg, GREEN, NORM);
  }
  else {
    printf("| %15s  %30s: %s \t\tFAILED %s|\n", test, arg , RED, NORM);
  }
}


/* TEST specified lock type with n threads 
   uses the parallel counter programs */
int TESTcounter(int type, int n) 
{
  int i;
  for (i = 0; i<trials; i++) {
    if (!parallel_time(10, n, type, 1)) {
      printf("Error: time counter returned incorrect value\n");
      return 0;
    }
    if (!parallel_work(1024, n, type)) {
      printf("Error: work counter returned incorrect value\n");
      return 0;
    }
  }
  return 1;
}


int TESTspack(int n)
{
  //long *fingerprint = (long *)malloc(sizeof(long)*n);
  long fingerprint;

  fingerprint = serial_pack(10, n, 100, 1, 1);
  if (!fingerprint) {
    printf("Error: serial packets dequeued more packets than enqueued\n");
    return 0;
  }
  return 1;
}


int TESTpack(int type, int n, int S)
{
  int i;
  long counter;

  for (i = 0; i < trials; i++) {
    counter = parallel_pack(200, n, 1000, 1, 1, 8, type, S);
    if (!counter) {
      printf("Error: parallel packets dequeued more packets than enqueued\n");
      return 0;
    }
  }
  return 1;
}

int TESTdistribution(int n)
{
  //int i;
  return 0;
}

int main() 
{
  trials = 1;

  printf("\nRunning counter tests (both work and time):\n\n");
  /*
  res(TESTcounter(1, 1), "TAS 1", "(n = 1)");
  res(TESTcounter(1, 16), "TAS 2", "(n = 16)");
  res(TESTcounter(2, 1), "BACKOFF 1", "(n = 1)");
  res(TESTcounter(2, 16), "BACKOFF 2", "(n = 16)");
  res(TESTcounter(3, 1), "MUTEX 1", "(n = 1)");
  res(TESTcounter(3, 16), "MUTEX 2", "(n = 16)");
  res(TESTcounter(4, 1), "ANDERSON 1", "(n = 1)");
  res(TESTcounter(4, 16), "ANDERSON 2", "(n = 16)");
  res(TESTcounter(5, 1), "CLH 1", "(n = 1)");
  res(TESTcounter(5, 16), "CLH 2", "(n = 16)");
  */
  printf("\nRunning packets tests:\n\n");

  res(TESTspack(1), "SERIAL 1", "(n = 1)");
  res(TESTspack(4), "SERIAL 2", "(n = 4)");
  printf("---\n");/*
  res(TESTpack(1, 1, 1), "LOCKFREE 1", "(S = 1, n = 1)");
  res(TESTpack(1, 16, 1), "LOCKFREE 2", "(S = 1, n = 16)");
  res(TESTpack(2, 1, 1), "LOCKFREE 1", "(S = 1, n = 1)");
  res(TESTpack(2, 16, 1), "LOCKFREE 2", "(S = 1, n = 16)");
  res(TESTpack(3, 1, 1), "LOCKFREE 1", "(S = 1, n = 1)");
  res(TESTpack(3, 16, 1), "LOCKFREE 2", "(S = 1, n = 16)");
  res(TESTpack(4, 1, 1), "LOCKFREE 1", "(S = 1, n = 1)");
  res(TESTpack(4, 16, 1), "LOCKFREE 2", "(S = 1, n = 16)");
  res(TESTpack(5, 1, 1), "LOCKFREE 1", "(S = 1, n = 1)");
  res(TESTpack(5, 16, 1), "LOCKFREE 2", "(S = 1, n = 16)");
  printf("---\n"); */
  res(TESTpack(1, 1, 2), "HOMEQ__TAS 1", "(L = 1, S = 2, n = 1)");
  res(TESTpack(1, 16, 2), "HOMEQ__TAS 2", "(L = 1, S = 2, n = 16)");
  res(TESTpack(2, 1, 2), "HOMEQ_BACK 1", "(L = 2, S = 2, n = 1)");
  res(TESTpack(2, 16, 2), "HOMEQ_BACK 2", "(L = 2, S = 2, n = 16)");
  res(TESTpack(3, 1, 2), "HOMEQ_MUTX 1", "(L = 3, S = 2, n = 1)");
  res(TESTpack(3, 16, 2), "HOMEQ_MUTX 2", "(L = 3, S = 2, n = 16)");
  res(TESTpack(4, 1, 2), "HOMEQ_ANDS 1", "(L = 4, S = 2, n = 1)");
  res(TESTpack(4, 16, 2), "HOMEQ_ANDS 2", "(L = 4, S = 2, n = 16)");
  res(TESTpack(5, 1, 2), "HOMEQ__CLH 1", "(L = 5, S = 2, n = 1)");
  res(TESTpack(5, 16, 2), "HOMEQ__CLH 2", "(L = 5, S = 2, n = 16)");
  printf("---\n"); /*
  res(TESTpack(1, 1, 3), "RANDQ__TAS 1", "(L = 1, S = 3, n = 1)");
  res(TESTpack(1, 16, 3), "RANDQ__TAS 2", "(L = 1, S = 3, n = 16)");
  res(TESTpack(2, 1, 3), "RANDQ_BACK 1", "(L = 2, S = 3, n = 1)");
  res(TESTpack(2, 16, 3), "RANDQ_BACK 2", "(L = 2, S = 3, n = 16)");
  res(TESTpack(3, 1, 3), "RANDQ_MUTX 1", "(L = 3, S = 3, n = 1)");
  res(TESTpack(3, 16, 3), "RANDQ_MUTX 2", "(L = 3, S = 3, n = 16)");
  res(TESTpack(4, 1, 3), "RANDQ_ANDS 1", "(L = 4, S = 3, n = 1)");
  res(TESTpack(4, 16, 3), "RANDQ_ANDS 2", "(L = 4, S = 3, n = 16)");
  res(TESTpack(5, 1, 3), "RANDQ__CLH 1", "(L = 5, S = 3, n = 1)");
  res(TESTpack(5, 16, 3), "RANDQ__CLH 2", "(L = 5, S = 3, n = 16)");
  printf("---\n");*/
  res(TESTpack(1, 1, 4), "LASTQ__TAS 1", "(L = 1, S = 4, n = 1)");
  res(TESTpack(1, 16, 4), "LASTQ__TAS 2", "(L = 1, S = 4, n = 16)");
  res(TESTpack(2, 1, 4), "LASTQ_BACK 1", "(L = 2, S = 4, n = 1)");
  res(TESTpack(2, 16, 4), "LASTQ_BACK 2", "(L = 2, S = 4, n = 16)");
  res(TESTpack(3, 1, 4), "LASTQ_MUTX 1", "(L = 3, S = 4, n = 1)");
  res(TESTpack(3, 16, 4), "LASTQ_MUTX 2", "(L = 3, S = 4, n = 16)");
  res(TESTpack(4, 1, 4), "LASTQ_ANDS 1", "(L = 4, S = 4, n = 1)");
  res(TESTpack(4, 16, 4), "LASTQ_ANDS 2", "(L = 4, S = 4, n = 16)");
  res(TESTpack(5, 1, 4), "LASTQ__CLH 1", "(L = 5, S = 4, n = 1)");
  res(TESTpack(5, 16, 4), "LASTQ__CLH 2", "(L = 5, S = 4, n = 16)");
  printf("---\n");
  return 0;
}
