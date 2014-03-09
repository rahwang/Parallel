#include "../src/Utils/hashpackettest.h"
#include "../src/Utils/hashtable.h"

#define GREEN "\033[0;32m"
#define NORM "\033[0m"
#define RED "\033[0;31m"

HashPacketGenerator_t * genSource;


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


/* TEST serial hash table type */
int TESTserial() {
  return serialHashPacketTest(100, .25, .25, .5, 4, 1000, 1);
}


/* Compare two hash lists
 * returns 1 if equivalent
 */
int compList(HashList_t *a, HashList_t *b) {
  HashItem_t *cur1 = a->head;
  HashItem_t *cur2 = b->head;

  while((cur1) && (cur2)) {
    if (cur1->value != cur2->value) {
      return 0;
    }
    if (cur1 && cur2) {
      cur1 = cur1->next;
      cur2 = cur2->next;
    }
  }
  return 1;
}


/* TEST enqueue()
 * should produce same list as manually enqueued control list
 * return 1 on PASS
 */
int TESTenqueue(int numPackets) {

  int i;
  HashPacketGenerator_t * source = createHashPacketGenerator(.25, .25, 0.5, 1000);

  HashList_t *queue = createHashList();
  HashList_t *test = createHashList();

  for (i=0; i < numPackets; i++) {
    volatile HashPacket_t *packet = getRandomPacket(source);
    if (i < 12) {
      add_hashlist(test, i, packet);
    }
    enqueue(queue, 12, packet, i);
  }

  return compList(queue, test);
}


/* TEST dequeue()
 * should produce same list as maually dequeued control list
 * return 1 on PASS
 */
int TESTdequeue(int n) {

  int i;
  HashPacketGenerator_t * source = createHashPacketGenerator(.25, .25, 0.5, 1000);

  HashList_t *queue = createHashList();
  HashList_t *test = createHashList();

  for (i=0; i < n; i++) {
    volatile HashPacket_t *packet = getRandomPacket(source);
    enqueue(test, 12, packet, i);
    enqueue(queue, 12, packet, i);
   }

  for (i = 0; i < n; i++) {
    dequeue(queue);
    remove_hashlist(test, i);
    if (!compList(queue, test)) {
      return 0;
    } 
  } 
  return 1;
}


int TESTcreatetable(int tableType, int numPkt, int numWorkers) {
  hashtable_t *t = (hashtable_t *)malloc(sizeof(hashtable_t));

  switch(tableType) {
  case(LOCKED):
    t->locked = createLockedTable(numPkt, 4, numWorkers);
    break;
  }

  HashPacketGenerator_t * source = createHashPacketGenerator(.25, .25, 0.5, 1000);
  int i = 0;
  int sum = 0;
  volatile HashPacket_t *pkt;
  for (i=0; i < numPkt; i++) {
    pkt = getAddPacket(source);
    add_locked(t, mangleKey((HashPacket_t *)pkt), pkt->body);
  }

  switch(tableType) {
  case(LOCKED):
    for (i = 0; i < (t->locked->size); i++) {
      if ((t->locked->table)[i] != NULL) {
	sum +=  (t->locked->table)[i]->size;
      }
    }
    i = (t->locked->size == (1 << numPkt)) && (t->locked->maxBucketSize == 4) && (sum == numPkt) && (t->locked->logSize == log2(t->locked->size));
    free_htable(t, tableType);
  }
  return i;
}


void addWorker(ParallelPacketWorker_t *data) {
  volatile HashPacket_t * pkt;
  //HashList_t **queues = data->queues;
  hashtable_t *table = data->table;
  //print_locked(table->locked);
  
  void (*addf)(hashtable_t *, int, volatile Packet_t *) = data->addf;
  while (data->myCount) {
    //printf("FROM THREAD %i, count = %i\n", data->tid, data->myCount);
    pkt = getRandomPacket(genSource);
    (*addf)(table, mangleKey((HashPacket_t *)pkt), pkt->body);
    (data->myCount)--;
  }
}


int TESTadd(int tableType, int numPkt, int numWorkers) 
{
  int i, rc;  
  if (numPkt % numWorkers) {
    fprintf(stderr,"ERROR: pkts not divisible by workers\n");
    exit(-1);
  }
  
  // allocate and initialize queues + fingerprints
  HashList_t *queues[numWorkers];
  long fingerprints[numWorkers];
  for (i = 0; i < numWorkers; i++) {
    queues[i] = createHashList();
    fingerprints[i] = 0;
  }
  
  genSource = createHashPacketGenerator(.25, .25, 0.5, 1000);

  // Initialize Table + worker data
  pthread_t worker[numWorkers];
  ParallelPacketWorker_t data[numWorkers];
  hashtable_t *htable = initTable(4, 0, data, genSource, numWorkers, NULL, queues, fingerprints, tableType);
  
  for (i = 0 ; i < numWorkers; i++) {
    data[i].myCount = numPkt/numWorkers;
  }

  // Spawn Workers
  for (i = 0; i <numWorkers; i++) {
    if ((rc = pthread_create(worker+i, NULL, (void *) &addWorker, (void *) (data+i)))) {
      fprintf(stderr,"ERROR: return code from pthread_create() for thread is %d\n", rc);
      exit(-1);
    }
  }
  
  // call join for each Worker
  for (i = 0; i < numWorkers; i++) {
    pthread_join(worker[i], NULL);
  }

  int res = (numPkt == countPkt(htable, tableType));
  if (!res) {
    print_locked(htable->locked);
    printf("size %i == packets %i ?\n", countPkt(htable, tableType), numPkt);
  }
  //print_locked(htable->locked);
  free_htable(htable, tableType);
  return res;
}


int TESTremove(int tableType, int numPkt, int numWorkers) 
{
  int i, rc;  
  if (numPkt % numWorkers) {
    fprintf(stderr,"ERROR: pkts not divisible by workers\n");
    exit(-1);
  }
  
  // allocate and initialize queues + fingerprints
  HashList_t *queues[numWorkers];
  long fingerprints[numWorkers];
  for (i = 0; i < numWorkers; i++) {
    queues[i] = createHashList();
    fingerprints[i] = 0;
  }
  
  genSource = createHashPacketGenerator(.25, .25, 0.5, 1000);

  // Initialize Table + worker data
  pthread_t worker[numWorkers];
  ParallelPacketWorker_t data[numWorkers];
  hashtable_t *htable = initTable(4, 0, data, genSource, numWorkers, NULL, queues, fingerprints, tableType);
  
  for (i = 0 ; i < numWorkers; i++) {
    data[i].myCount = numPkt/numWorkers;
  }
  
  // Spawn Workers
  for (i = 0; i <numWorkers; i++) {
    if ((rc = pthread_create(worker+i, NULL, (void *) &addWorker, (void *) (data+i)))) {
      fprintf(stderr,"ERROR: return code from pthread_create() for thread is %d\n", rc);
      exit(-1);
    }
  }
  
  // call join for each Worker
  for (i = 0; i < numWorkers; i++) {
    pthread_join(worker[i], NULL);
  }
  
  int res = (numPkt == countPkt(htable, tableType));
 
  if (!res) {
    print_locked(htable->locked);
    printf("size %i == packets %i ?\n", countPkt(htable, tableType), numPkt);
  }
  free_htable(htable, tableType);
  return res;
}


int main()
{
  //int trials = 1; 
  printf("\nRunning Framework Tests\n");
  printf("---\n");
  res(TESTserial(), "SERIAL", "(n = 1)");
  res(TESTenqueue(12), "ENQUEUE", "(numPkt = 12)");
  res(TESTdequeue(12), "DEQUEUE", "(numPkt = 12)");
  printf("---\n");
  printf("\nRunning Locked Table Tests\n");
  printf("---\n");
  res(TESTcreatetable(1, 1, 1), "CREATE", "(sz = 1, n = 1)");
  res(TESTcreatetable(1, 16, 1), "CREATE", "(sz = 16, n = 1)");
  res(TESTcreatetable(1, 16, 16), "CREATE", "(sz = 16, n = 16)"); 
  res(TESTadd(1, 1, 1), "ADD", "(pkts = 1, n = 1)"); 
  res(TESTadd(1, 8, 1), "ADD", "(pkts = 8, n = 1)");  
  res(TESTadd(1, 16, 1), "ADD", "(pkts = 16, n = 1)");
  res(TESTadd(1, 8, 2), "ADD", "(pkts = 8, n = 2)");  
  res(TESTadd(1, 16, 2), "ADD", "(pkts = 16, n = 2)"); 
  res(TESTadd(1, 64, 16), "ADD", "(pkts = 64, n = 16)"); 
  printf("---\n");
  return 0;
}