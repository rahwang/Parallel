#include "../src/Utils/hashpackettest.h"
#include "../src/Utils/hashtable.h"

#define GREEN "\033[0;32m"
#define NORM "\033[0m"
#define RED "\033[0;31m"

#define o_reads 1
#define o_writes 2
#define s_reads 3
#define s_writes 4

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
  return serialHashPacketTest(1000, .25, .25, .5, 4, 4000, 4);
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
  HashItem_t * newItem;
  HashPacketGenerator_t * source = createHashPacketGenerator(.25, .25, 0.5, 1000);

  HashList_t *queue = createHashList();
  volatile HashPacket_t *packet = getRandomPacket(source);
  newItem = (HashItem_t *)malloc(sizeof(HashItem_t));
  newItem->key = numPackets;
  newItem->value = packet;
  queue->head = newItem;
  queue->tail = newItem;
  queue->size++;
  
  for (i=0; i < numPackets-1; i++) {
    packet = getRandomPacket(source);
    enqueue(queue, numPackets+1, packet, i);
  }
  return (queue->size == numPackets);
}


/* TEST dequeue()
 * should produce same list as maually dequeued control list
 * return 1 on PASS
 */
int TESTdequeue(int n) {

  int i;
  HashItem_t * newItem;
  HashPacketGenerator_t * source = createHashPacketGenerator(.25, .25, 0.5, 1000);

  HashList_t *queue = createHashList();
  volatile HashPacket_t *packet = getRandomPacket(source);
  newItem = (HashItem_t *)malloc(sizeof(HashItem_t));
  newItem->key = n;
  newItem->value = packet;
  queue->head = newItem;
  queue->tail = newItem;
  queue->size++;


  for (i=0; i < n-1; i++) {
    volatile HashPacket_t *packet = getRandomPacket(source);
    enqueue(queue, n+1, packet, i);
   }

  int size = n;
  for (i = 0; i < n-1; i++) {
    dequeue(queue);
    size--;
    if (size != queue->size) {
      printf("size = %i\n", queue->size);
      return 0;
    }
  } 
  return 1;
}


int TESTcreatetable(int tableType, int numPkt, int numWorkers) {
  hashtable_t *t = (hashtable_t *)malloc(sizeof(hashtable_t));

  HashPacketGenerator_t * source = createHashPacketGenerator(.25, .25, 0.5, 1000);
  int size, i = 0;
  int sum = 0;
  volatile HashPacket_t *pkt;

  switch(tableType) {
  case(LOCKED):
    t->locked = createLockedTable(numPkt, 12, numWorkers);
    for (i=0; i < numPkt; i++) {
      pkt = getAddPacket(source);
      add_locked(t, mangleKey((HashPacket_t *)pkt), pkt->body);
    }
    size = t->locked->size;
    for (i = 0; i < size; i++) {
      if ((t->locked->table)[i] != NULL) {
	sum +=  (t->locked->table)[i]->size;
      }
    }
    break;
  case(LOCKFREEC):
    t->lockFreeC = createLockFreeCTable(numPkt, 12, numWorkers);
    for (i=0; i < numPkt; i++) {
      pkt = getAddPacket(source);
      add_lockFreeC(t, mangleKey((HashPacket_t *)pkt), pkt->body);
    }
    size = t->lockFreeC->size;
    for (i = 0; i < size; i++) {
      if ((t->lockFreeC->table)[i] != NULL) {
	sum +=  (t->lockFreeC->table)[i]->size;
      }
    }
    break; 
  case(LINEARPROBED):
    t->linearProbe = createLinearProbeTable(numPkt, 12, numWorkers);
    for (i=0; i < numPkt; i++) {
      pkt = getAddPacket(source);
      add_linearProbe(t, mangleKey((HashPacket_t *)pkt), pkt->body);
    }
    size = t->linearProbe->size;
    for (i = 0; i < size; i++) {
      sum += ((t->linearProbe->table)[i].value != NULL);
    }
    break;
  case(AWESOME):
    t->awesome = createAwesomeTable(30);
    for (i=0; i < numPkt; i++) {
      pkt = getAddPacket(source);
      add_awesome(t, i, pkt->body);
    }
    size = (1 << numPkt);
    //size = t->awesome->bucketSize;
    sum = (int)countPkt(t, tableType);
    break; 
  }
  if (size != (1 << numPkt)) {
    fprintf(stderr,"ERROR: Tree size incorrect. Got %i. Want %i.\n", size, (1 << numPkt));
    return 0;
  }
  if (sum != numPkt) {
    fprintf(stderr,"ERROR: Number of entries incorrect. Got %i. Want %i.\n", sum, numPkt);
      return 0;
  }
  free_htable(t, tableType);
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
    (*addf)(table, (data->myCount)+(data->tid * 100), pkt->body);
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
  hashtable_t *htable = initTable(12, 0, data, genSource, numWorkers, NULL, queues, fingerprints, tableType);
  
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
    print_table(htable, tableType);
    printf("size %li == packets %i ?\n", countPkt(htable, tableType), numPkt);
  }
  //print_table(htable, tableType);
  free_htable(htable, tableType);
  return res;
}


void containsWorker(ParallelPacketWorker_t *data) {
  //volatile HashPacket_t * pkt;
  //HashList_t **queues = data->queues;
  hashtable_t *table = data->table;
  //print_locked(table->locked);
  int top = data->myCount;

  bool (*containsf)(hashtable_t *, int) = data->containsf;

  while (data->myCount > 0) {
    //printf("Hello %li\n", data->myCount);
    if (!((*containsf)(table, (data->myCount)-1)) 
	|| ((*containsf)(table, (data->myCount)+top))) {
	data->myCount = -1;
	break;
    } 
    (data->myCount)--;
  }
}


int TESTcontains(int tableType, int numPkt, int numWorkers)
{
  int i, rc;  
  // allocate and initialize queues + fingerprints
  HashList_t *queues[numWorkers];
  long fingerprints[numWorkers];
  volatile HashPacket_t *pkt;
  for (i = 0; i < numWorkers; i++) {
    queues[i] = createHashList();
    pkt = getRandomPacket(genSource);
    HashItem_t *newItem = (HashItem_t *)malloc(sizeof(HashItem_t));
    newItem->key = numPkt;
    newItem->value = pkt;
    queues[i]->head = newItem;
    queues[i]->tail = newItem;
    queues[i]->size++;

    fingerprints[i] = 0;
  }
  
  genSource = createHashPacketGenerator(.25, .25, 0.5, 1000);

  // Initialize Table + worker data
  pthread_t worker[numWorkers];
  ParallelPacketWorker_t data[numWorkers];
  hashtable_t *htable = initTable(12, 0, data, genSource, numWorkers, NULL, queues, fingerprints, tableType);

  for (i = 0 ; i < numWorkers; i++) {
    data[i].myCount = numPkt;
  }

  i = 0;

  switch(tableType) {
  case(LOCKED):
    for (i=0; i < numPkt; i++) {
      pkt = getRandomPacket(genSource);
      add_locked(htable, i, pkt->body);
      enqueue(queues[0], numPkt+1, pkt, i);
    }
    break;
  case(LOCKFREEC):
    for (i=0; i < numPkt; i++) {
      pkt = getRandomPacket(genSource);
      add_lockFreeC(htable, i, pkt->body);
      enqueue(queues[0], numPkt+1, pkt, i);
    }
    break;
  case(LINEARPROBED):
    for (i=0; i < numPkt; i++) {
      pkt = getRandomPacket(genSource);
      add_linearProbe(htable, i, pkt->body);
      enqueue(queues[0], numPkt+1, pkt, i);
    }
    break;
  case(AWESOME):
    for (i=0; i < numPkt; i++) {
      pkt = getRandomPacket(genSource);
      add_awesome(htable, i, pkt->body);
      enqueue(queues[0], numPkt+1, pkt, i);
    }
    break;
  }
  
  // Spawn Workers
  for (i = 0; i <numWorkers; i++) {
    if ((rc = pthread_create(worker+i, NULL, (void *) &containsWorker, (void *) (data+i)))) {
      fprintf(stderr,"ERROR: return code from pthread_create() for thread is %d\n", rc);
      exit(-1);
    }
  }
  
  // call join for each Worker
  for (i = 0; i < numWorkers; i++) {
    pthread_join(worker[i], NULL);
  }   
  
  int res = 0;
  // -1 indicates failure. Check all threads
  for (i = 0; i < numWorkers; i++) {
    res += (data[i].myCount == -1);
    //printf("COUNT = %i\n", data[i].myCount);
  }
  //print_table(htable, tableType);
  free_htable(htable, tableType);
  return !res;
}


void removeWorker(ParallelPacketWorker_t *data) {
  volatile HashPacket_t * pkt;
  //HashList_t **queues = data->queues;
  hashtable_t *table = data->table;
  //print_locked(table->locked);

  bool (*removef)(hashtable_t *, int) = data->removef;


  while (data->myCount > 0) {
    pkt = getPacket(data->queues, data->tid);
    if (!(*removef)(table, pkt->key)) {
      data->myCount = -1;
      return;
    } 
    (data->myCount)--;
  }
}


int TESTremove(int tableType, int numPkt, int numWorkers) 
{
  int i, rc;  
  volatile HashPacket_t *pkt;
  if (numPkt % numWorkers) {
    fprintf(stderr,"ERROR: pkts not divisible by workers\n");
    exit(-1);
  }
  
  genSource = createHashPacketGenerator(.25, .25, 0.5, 1000);

  // allocate and initialize queues + fingerprints

  HashList_t *queues[numWorkers];
  long fingerprints[numWorkers];
  for (i = 0; i < numWorkers; i++) {
    queues[i] = createHashList();
    fingerprints[i] = 0;
  }

  // Initialize Table + worker data
  pthread_t worker[numWorkers];
  ParallelPacketWorker_t data[numWorkers];
  hashtable_t *htable = initTable(4, 0, data, genSource, numWorkers, NULL, queues, fingerprints, tableType);

  switch(tableType) {
  case(LOCKED):
    for (i=0; i < (numPkt*2); i++) {
      pkt = getRandomPacket(genSource);
      pkt->key = i;
      add_locked(htable, pkt->key, pkt->body);
      enqueue(queues[i % numWorkers], numPkt+1, pkt, i);
    }
    break;
  case(LOCKFREEC):
    for (i=0; i < (numPkt*2); i++) {
      pkt = getRandomPacket(genSource);
      pkt->key = i;
      add_lockFreeC(htable, pkt->key, pkt->body);
      enqueue(queues[i % numWorkers], numPkt+1, pkt, i);
    }
    break; 
  case(LINEARPROBED):
    for (i=0; i < (numPkt*2); i++) {
      pkt = getRandomPacket(genSource);
      pkt->key = i;
      add_linearProbe(htable, pkt->key, pkt->body);
      enqueue(queues[i % numWorkers], numPkt+1, pkt, i);
    }
    break; 
  case(AWESOME):
    for (i=0; i < (numPkt*2); i++) {
	pkt = getRandomPacket(genSource);
	pkt->key = i;
	add_awesome(htable, pkt->key, pkt->body);
	enqueue(queues[i % numWorkers], numPkt+1, pkt, i);
    }
    break;
  }
  
  for (i = 0 ; i < numWorkers; i++) {
    data[i].myCount = numPkt/numWorkers;
  }
  
  // Spawn Workers
  for (i = 0; i <numWorkers; i++) {
    if ((rc = pthread_create(worker+i, NULL, (void *) &removeWorker, (void *) (data+i)))) {
      fprintf(stderr,"ERROR: return code from pthread_create() for thread is %d\n", rc);
      exit(-1);
    }
  }
  
  // call join for each Worker
  for (i = 0; i < numWorkers; i++) {
    pthread_join(worker[i], NULL);
  }

  int res = 0;
  for (i = 0; i < numWorkers; i++) {
    res += (data[i].myCount == -1);
    //printf("COUNT = %li\n", data[i].myCount);
  }

  //print_locked(htable->locked);
  //printf("size = %li\n",  countPkt(htable, tableType));
  int size = (countPkt(htable, tableType) == numPkt); 
  free_htable(htable, tableType);
  return (!res) && size;
}


int TESTintegration(int tableType, int time, int numWorkers) {
  parallelHashPacketTest(time, .25, .25, .5, 12, 4000, 4, numWorkers, tableType);
  return 1;
}

void speedups(int time, int n, int testType) {
  float adds = 0.09;
  float rems = 0.01;
  float hit = 0.5;
  float s = 16;
  int b = 12;
  char *type;

  switch(testType) {
  case(o_reads):
    adds = 0.09;
    rems = 0.01;
    hit = 0.9;
    type = "reads";
    break;
  case(o_writes):
    adds = 0.45;
    rems = 0.05;
    hit = 0.9;
    type = "writes";
    break;
  case(s_reads):
    adds = 0.09;
    rems = 0.01;
    hit = 0.75;
    type = "reads";
    break;
  case(s_writes):
    adds = 0.45;
    rems = 0.05;
    hit = 0.75;
    type = "writes";
    break;
  }

  double serial = serialHashPacketTest(time, adds, rems, hit, b, 4000, s);
  double parallel;

  printf("\n~~~~ Speedups (n = %i, type = %s) ~~~~ \n\n", n, type); 
  printf("Serial packets = %15f\n\n", serial); 
  
  parallel = parallelHashPacketTest(time, adds, rems, hit, b, 4000, s, n, 1);
  printf("LOCKED = %15f\n", parallel/serial); 
  parallel = parallelHashPacketTest(time, adds, rems, hit, b, 4000, s, n, 2);
  printf("LOCKFC = %15f\n", parallel/serial);
  parallel = parallelHashPacketTest(time, adds, rems, hit, b, 4000, s, n, 3);
  printf("LINEAR = %15f\n", parallel/serial); 
  parallel = parallelHashPacketTest(time, adds, rems, hit, b, 4000, s, n, 4);
  printf("AWESOM = %15f\n", parallel/serial); 
}


void dispatcher(int time, int n, int testType) {
  float adds = 0.09;
  float rems = 0.01;
  float hit = 0.5;
  float s = 16;
  int b = 12;
  char *type;
  int W = 8000;

  switch(testType) {
  case(o_reads):
    adds = 0.09;
    rems = 0.01;
    hit = 0.9;
    type = "reads";
    break;
  case(o_writes):
    adds = 0.45;
    rems = 0.05;
    hit = 0.9;
    type = "writes";
    break;
  case(s_reads):
    adds = 0.09;
    rems = 0.01;
    hit = 0.75;
    type = "reads";
    break;
  case(s_writes):
    adds = 0.45;
    rems = 0.05;
    hit = 0.75;
    type = "writes";
    break;
  }

  double dis = noloadHashPacketTest(time, adds, rems, hit, b, W, s, n, 1);
  double parallel;

  printf("\n~~~~ Speedups (n = %i, type = %s) ~~~~ \n\n", n, type); 
  printf("Dispatcher packets = %15f\n\n", dis); 

  parallel = parallelHashPacketTest(time, adds, rems, hit, b, W, s, n, 1);
  printf("LOCKED = %15f\n", parallel/dis); 
  parallel = parallelHashPacketTest(time, adds, rems, hit, b, W, s, n, 2);
  printf("LOCKFC = %15f\n", parallel/dis);
  parallel = parallelHashPacketTest(time, adds, rems, hit, b, W, s, n, 3);
  printf("LINEAR = %15f\n", parallel/dis); 
  parallel = parallelHashPacketTest(time, adds, rems, hit, b, W, s, n, 4);
  printf("AWESOM = %15f\n", parallel/dis); 
}


int main()
{ 
  printf("\nRunning Framework Tests\n");
  printf("---\n");
  res(TESTserial(), "SERIAL", "(n = 1)");
  res(TESTenqueue(12), "ENQUEUE", "(numPkt = 12)");
  res(TESTdequeue(12), "DEQUEUE", "(numPkt = 12)");
  printf("---\n"); 
  printf("\nRunning Locked Table Tests\n");
  printf("---\n");
  res(TESTcreatetable(1, 1, 1), "CREATE", "(logSz = 1, n = 1)");
  res(TESTcreatetable(1, 16, 1), "CREATE", "(logSz = 16, n = 1)");
  res(TESTcreatetable(1, 16, 32), "CREATE", "(logSz = 16, n = 32)");  
  res(TESTadd(1, 1, 1), "ADD", "(pkts = 1, n = 1)"); 
  res(TESTadd(1, 16, 1), "ADD", "(pkts = 16, n = 1)");
  res(TESTadd(1, 16, 16), "ADD", "(pkts = 16, n = 16)"); 
  res(TESTadd(1, 128, 32), "ADD", "(pkts = 64, n = 32)"); 
  res(TESTcontains(1, 1, 1), "CONTAINS", "(pkts = 1, n = 1)"); 
  res(TESTcontains(1, 16, 1), "CONTAINS", "(pkts = 16, n = 1)");
  res(TESTcontains(1, 16, 16), "CONTAINS", "(pkts = 16, n = 16)");
  res(TESTcontains(1, 128, 32), "CONTAINS", "(pkts = 128, n = 32)");
  res(TESTremove(1, 1, 1), "REMOVE", "(pkts = 1, n = 1)");
  res(TESTremove(1, 16, 1), "REMOVE", "(pkts = 16, n = 1)");
  res(TESTremove(1, 16, 16), "REMOVE", "(pkts = 16, n = 16)");
  res(TESTremove(1, 128, 32), "REMOVE", "(pkts = 64, n = 32)"); 
  res(TESTintegration(1, 2000, 1), "INTEGRATION", "(time = 2000, n = 1)");
  res(TESTintegration(1, 2000, 16), "INTEGRATION", "(time = 2000, n = 16)"); 
  res(TESTintegration(1, 2000, 32), "INTEGRATION", "(time = 2000, n = 32)"); 
  printf("---\n"); 
  printf("\nRunning Lock Free Contains Table Tests\n");
  printf("---\n"); 
  res(TESTcreatetable(2, 1, 1), "CREATE", "(logSz = 1, n = 1)");
  res(TESTcreatetable(2, 16, 1), "CREATE", "(logSz = 16, n = 1)");
  res(TESTcreatetable(2, 16, 32), "CREATE", "(logSz = 16, n = 32)");  
  res(TESTadd(2, 1, 1), "ADD", "(pkts = 1, n = 1)"); 
  res(TESTadd(2, 16, 1), "ADD", "(pkts = 16, n = 1)");
  res(TESTadd(2, 16, 16), "ADD", "(pkts = 16, n = 16)"); 
  res(TESTadd(2, 128, 32), "ADD", "(pkts = 64, n = 32)"); 
  res(TESTcontains(2, 1, 1), "CONTAINS", "(pkts = 1, n = 1)"); 
  res(TESTcontains(2, 16, 1), "CONTAINS", "(pkts = 16, n = 1)");
  res(TESTcontains(2, 16, 16), "CONTAINS", "(pkts = 16, n = 16)");
  res(TESTcontains(2, 128, 32), "CONTAINS", "(pkts = 128, n = 32)");
  res(TESTremove(2, 1, 1), "REMOVE", "(pkts = 1, n = 1)");
  res(TESTremove(2, 16, 1), "REMOVE", "(pkts = 16, n = 1)");
  res(TESTremove(2, 16, 16), "REMOVE", "(pkts = 16, n = 16)");
  res(TESTremove(2, 128, 32), "REMOVE", "(pkts = 64, n = 32)"); 
  res(TESTintegration(2, 2000, 1), "INTEGRATION", "(time = 2000, n = 1)");
  res(TESTintegration(2, 2000, 16), "INTEGRATION", "(time = 2000, n = 16)"); 
  res(TESTintegration(2, 2000, 32), "INTEGRATION", "(time = 2000, n = 32)"); 
  printf("---\n"); 
  printf("\nRunning Linear Probed Table Tests\n");
  printf("---\n"); 
  res(TESTcreatetable(3, 1, 1), "CREATE", "(logSz = 1, n = 1)");
  res(TESTcreatetable(3, 16, 1), "CREATE", "(logSz = 4, n = 1)");
  res(TESTcreatetable(3, 16, 32), "CREATE", "(logSz = 8, n = 32)"); 
  res(TESTadd(3, 1, 1), "ADD", "(pkts = 1, n = 1)"); 
  res(TESTadd(3, 16, 1), "ADD", "(pkts = 16, n = 1)");
  res(TESTadd(3, 16, 16), "ADD", "(pkts = 16, n = 16)"); 
  res(TESTadd(3, 128, 32), "ADD", "(pkts = 128, n = 32)"); 
  res(TESTcontains(3, 1, 1), "CONTAINS", "(pkts = 1, n = 1)"); 
  res(TESTcontains(3, 16, 1), "CONTAINS", "(pkts = 16, n = 1)");
  res(TESTcontains(3, 16, 16), "CONTAINS", "(pkts = 16, n = 16)");
  res(TESTcontains(3, 128, 32), "CONTAINS", "(pkts = 128, n = 32)");
  res(TESTremove(3, 1, 1), "REMOVE", "(pkts = 1, n = 1)");
  res(TESTremove(3, 16, 1), "REMOVE", "(pkts = 16, n = 1)");
  res(TESTremove(3, 16, 16), "REMOVE", "(pkts = 16, n = 16)");
  res(TESTremove(3, 128, 32), "REMOVE", "(pkts = 128, n = 32)"); 
  res(TESTintegration(3, 2000, 1), "INTEGRATION", "(time = 2000, n = 1)");
  res(TESTintegration(3, 2000, 16), "INTEGRATION", "(time = 2000, n = 16)"); 
  res(TESTintegration(3, 2000, 32), "INTEGRATION", "(time = 2000, n = 32)"); 
  printf("---\n"); 
  printf("\nRunning Awesome Table Tests\n");
  printf("---\n"); 
  res(TESTcreatetable(4, 1, 1), "CREATE", "(logSz = 30, n = 1)");
  res(TESTcreatetable(4, 16, 1), "CREATE", "(logSz = 30, n = 1)");
  res(TESTcreatetable(4, 16, 32), "CREATE", "(logSz = 30, n = 32)"); 
  res(TESTadd(4, 1, 1), "ADD", "(pkts = 1, n = 1)"); 
  res(TESTadd(4, 16, 1), "ADD", "(pkts = 16, n = 1)");
  res(TESTadd(4, 16, 16), "ADD", "(pkts = 16, n = 16)"); 
  res(TESTadd(4, 128, 32), "ADD", "(pkts = 128, n = 32)"); 
  res(TESTcontains(4, 1, 1), "CONTAINS", "(pkts = 1, n = 1)"); 
  res(TESTcontains(4, 16, 1), "CONTAINS", "(pkts = 16, n = 1)");
  res(TESTcontains(4, 16, 16), "CONTAINS", "(pkts = 16, n = 16)");
  res(TESTcontains(4, 128, 32), "CONTAINS", "(pkts = 128, n = 32)");
  res(TESTremove(4, 1, 1), "REMOVE", "(pkts = 1, n = 1)");
  res(TESTremove(4, 16, 1), "REMOVE", "(pkts = 16, n = 1)");
  res(TESTremove(4, 16, 16), "REMOVE", "(pkts = 16, n = 16)");
  res(TESTremove(4, 128, 32), "REMOVE", "(pkts = 128, n = 32)"); 
  res(TESTintegration(4, 2000, 1), "INTEGRATION", "(time = 2000, n = 1)");
  res(TESTintegration(4, 2000, 16), "INTEGRATION", "(time = 2000, n = 16)"); 
  res(TESTintegration(4, 2000, 32), "INTEGRATION", "(time = 2000, n = 32)"); 
  printf("---\n");

  //res(TESTintegration(4, 3, 4), "INTEGRATION", "(time = 5, n = 1)");
 
  //speedups(2000, 1, o_reads);
  //speedups(2000, 1, o_writes);
  //speedups(2000, 2, o_reads);
  //speedups(2000, 2, o_writes);
  //speedups(2000, 4, o_reads);
  //speedups(2000, 4, o_writes);
  //speedups(2000, 7, o_reads);
  //speedups(2000, 7, o_writes);

 
  speedups(1000, 4, s_reads);
  speedups(1000, 4, s_writes);
  speedups(1000, 7, s_reads);
  speedups(1000, 7, s_writes);
  speedups(1000, 8, s_reads);
  speedups(1000, 8, s_writes);
  speedups(1000, 10, s_reads);
  speedups(1000, 10, s_writes);
  speedups(1000, 15, s_reads);
  speedups(1000, 15, s_writes); 
  dispatcher(2000, 1, s_reads);
  dispatcher(2000, 1, s_writes);
  dispatcher(2000, 3, s_reads);
  dispatcher(2000, 3, s_writes);
  dispatcher(2000, 7, s_reads);
  dispatcher(2000, 7, s_writes);
  dispatcher(2000, 15, s_reads);
  dispatcher(2000, 15, s_writes);

  //dispatcher(2000, 1, s_writes);

  
  return 0;
}
