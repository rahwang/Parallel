#include "../src/Utils/hashpackettest.h"
#include "../src/Utils/hashtable.h"

#define GREEN "\033[0;32m"
#define NORM "\033[0m"
#define RED "\033[0;31m"


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


int TESTcreatetable(int initSize, int type) {
  hashtable_t *t = (hashtable_t *)malloc(sizeof(hashtable_t));

  switch(type) {
  case(LOCKED):
    t->locked = createLockedTable(initSize*2, 4, 1);
    break;
  }

  printf("Try?\n");
  HashPacketGenerator_t * source = createHashPacketGenerator(.25, .25, 0.5, 1000);
  printf("Try?\n");
  int i = 0;
  int sum = 0;
  volatile HashPacket_t *pkt;
  for (i=0; i < initSize; i++) {
    pkt = getAddPacket(source);
    add_locked(t, mangleKey((HashPacket_t *)pkt), pkt->body);
    printf("Try?\n");
  }

  print_locked(t->locked);

  switch(type) {
  case(LOCKED):
    for (i = 0; i < initSize*2; i++) {
      sum += (t->locked->table)[i]->size;
    }
    i = (t->locked->size == initSize*2) && (t->locked->maxBucketSize == 4) && (sum == initSize);
  }
  free(t);
  return i;
}


int main()
{
  //int trials = 1;

  printf("\nRunning Framework Tests\n");
  printf("---\n");
  //res(TESTserial(), "SERIAL", "(n = 1)");
  //res(TESTenqueue(12), "ENQUEUE", "(numPkt = 12)");
  //res(TESTdequeue(12), "DEQUEUE", "(numPkt = 12)");
  printf("---\n");
  printf("\nRunning Framework Tests\n");
  printf("---\n");
  res(TESTcreatetable(2, 1), "CREATE", "(sz = 2)");
  res(TESTcreatetable(16, 1), "CREATE", "(sz = 16)");
 
  return 0;
}
