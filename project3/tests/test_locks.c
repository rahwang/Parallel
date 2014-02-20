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
  int sum = 0;
  for (i = 0; i<trials; i++) {
    sum = parallel_time(10, n, type);
    if (sum) {
      printf("Error: time counter returned incorrect value\n");
      return 0;
    }
    sum = parallel_work(1024, n, type);
    if (sum) {
      printf("Error: work counter returned incorrect value\n");
      return 0;
    }
  }
  return 1;
}


int TESTspack(int n)
{
  //long *fingerprint = (long *)malloc(sizeof(long)*n);
  long *fingerprint;

  fingerprint = serial_pack(10, n, 100, 1, 1);
  if (!fingerprint) {
    printf("Error: serial packets dequeued more packets than enqueued\n");
    free(fingerprint);
    return 0;
  }
  free(fingerprint);
  return 1;
}


int TESTpack(int type, int n, int S)
{
  int i;
  //long *fingerprint = (long *)malloc(sizeof(long)*n);
  pack_data_t *data;

  for (i = 0; i < trials; i++) {
    data = parallel_pack(10, n, 100, 1, 1, 8, type, S);
    if (!data) {
      printf("Error: parallel packets dequeued more packets than enqueued\n");
      free(data);
      return 0;
    }
    free(data);
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
  trials = 10;

  printf("\nRunning counter tests (both work and time):\n\n");

  res(TESTcounter(1, 1), "TAS 1", "(n = 1)");
  res(TESTcounter(1, 8), "TAS 2", "(n = 8)");
  res(TESTcounter(2, 1), "BACKOFF 1", "(n = 1)");
  res(TESTcounter(2, 8), "BACKOFF 2", "(n = 8)");
  res(TESTcounter(3, 1), "MUTEX 1", "(n = 1)");
  res(TESTcounter(3, 8), "MUTEX 2", "(n = 8)");
  res(TESTcounter(4, 1), "ANDERSON 1", "(n = 1)");
  res(TESTcounter(4, 8), "ANDERSON 2", "(n = 8)");
  res(TESTcounter(5, 1), "CLH 1", "(n = 1)");
  res(TESTcounter(5, 8), "CLH 2", "(n = 8)");

  printf("\nRunning packets tests:\n\n");

  res(TESTspack(1), "SERIAL 1", "(n = 1)");
  res(TESTspack(4), "SERIAL 2", "(n = 4)");
  printf("---\n");
  res(TESTpack(1, 1, 1), "LOCKFREE 1", "(S = 1, n = 1)");
  res(TESTpack(1, 8, 1), "LOCKFREE 2", "(S = 1, n = 8)");
  printf("---\n"); 
  res(TESTpack(1, 1, 2), "HOMEQ__TAS 1", "(L = 1, S = 2, n = 1)");
  res(TESTpack(1, 8, 2), "HOMEQ__TAS 2", "(L = 1, S = 2, n = 8)");
  res(TESTpack(2, 1, 2), "HOMEQ_BACK 1", "(L = 2, S = 2, n = 1)");
  res(TESTpack(2, 8, 2), "HOMEQ_BACK 2", "(L = 2, S = 2, n = 8)");
  res(TESTpack(3, 1, 2), "HOMEQ_MUTX 1", "(L = 3, S = 2, n = 1)");
  res(TESTpack(3, 8, 2), "HOMEQ_MUTX 2", "(L = 3, S = 2, n = 8)");
  res(TESTpack(4, 1, 2), "HOMEQ_ANDS 1", "(L = 4, S = 2, n = 1)");
  res(TESTpack(4, 8, 2), "HOMEQ_ANDS 2", "(L = 4, S = 2, n = 8)");
  res(TESTpack(5, 1, 2), "HOMEQ__CLH 1", "(L = 5, S = 2, n = 1)");
  res(TESTpack(5, 8, 2), "HOMEQ__CLH 2", "(L = 5, S = 2, n = 8)");
  printf("---\n");
  res(TESTpack(1, 1, 3), "RANDQ__TAS 1", "(L = 1, S = 3, n = 1)");
  res(TESTpack(1, 8, 3), "RANDQ__TAS 2", "(L = 1, S = 3, n = 8)");
  res(TESTpack(2, 1, 3), "RANDQ_BACK 1", "(L = 2, S = 3, n = 1)");
  res(TESTpack(2, 8, 3), "RANDQ_BACK 2", "(L = 2, S = 3, n = 8)");
  res(TESTpack(3, 1, 3), "RANDQ_MUTX 1", "(L = 3, S = 3, n = 1)");
  res(TESTpack(3, 8, 3), "RANDQ_MUTX 2", "(L = 3, S = 3, n = 8)");
  res(TESTpack(4, 1, 3), "RANDQ_ANDS 1", "(L = 4, S = 3, n = 1)");
  res(TESTpack(4, 8, 3), "RANDQ_ANDS 2", "(L = 4, S = 3, n = 8)");
  res(TESTpack(5, 1, 3), "RANDQ__CLH 1", "(L = 5, S = 3, n = 1)");
  res(TESTpack(5, 8, 3), "RANDQ__CLH 2", "(L = 5, S = 3, n = 8)");
  printf("---\n");
  res(TESTpack(1, 1, 4), "LASTQ__TAS 1", "(L = 1, S = 4, n = 1)");
  res(TESTpack(1, 8, 4), "LASTQ__TAS 2", "(L = 1, S = 4, n = 8)");
  res(TESTpack(2, 1, 4), "LASTQ_BACK 1", "(L = 2, S = 4, n = 1)");
  res(TESTpack(2, 8, 4), "LASTQ_BACK 2", "(L = 2, S = 4, n = 8)");
  res(TESTpack(3, 1, 4), "LASTQ_MUTX 1", "(L = 3, S = 4, n = 1)");
  res(TESTpack(3, 8, 4), "LASTQ_MUTX 2", "(L = 3, S = 4, n = 8)");
  res(TESTpack(4, 1, 4), "LASTQ_ANDS 1", "(L = 4, S = 4, n = 1)");
  res(TESTpack(4, 8, 4), "LASTQ_ANDS 2", "(L = 4, S = 4, n = 8)");
  res(TESTpack(5, 1, 4), "LASTQ__CLH 1", "(L = 5, S = 4, n = 1)");
  res(TESTpack(5, 8, 4), "LASTQ__CLH 2", "(L = 5, S = 4, n = 8)");
  printf("---\n");
  return 0;
}
