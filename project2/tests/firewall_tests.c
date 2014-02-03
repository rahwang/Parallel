#include <stdio.h>
#include <string.h>
#include "../src/funcs.h"

#define GREEN "\033[0;32m"
#define NORM "\033[0m"
#define RED "\033[0;31m"

#define DONE 1000000



/* Format test results into pretty print statement */
void res(int pass, char *test, char *arg) {
  if (pass) {
    printf("| %15s  %25s: %s \t\tPASSED %s|\n", test, arg, GREEN, NORM);
  }
  else {
    printf("| %15s  %25s: %s \t\tFAILED %s|\n", test, arg , RED, NORM);
  }
}



/* TEST create_queues() 
 * Should return array of SerialList pointers 
 * returns 1 on PASS 
*/
int TESTcreate() {
  int i, n = 10;
  SerialList_t **new = create_queues(n);
  for (i = 0; i < n; i++) {
    if (sizeof(new[i]) != sizeof(SerialList_t *)) {
      return 0;
    }
  }
  return 1;
}



/* Compare two serial lists
 * returns 1 if equivalent
 */
int compList(SerialList_t *a, SerialList_t *b) {
  Item_t *cur1 = a->head;
  Item_t *cur2 = b->head;

  while((cur1) || (cur2)) {
    if (cur1->value != cur2->value) {
      return 0;
    }
    cur1 = cur1->next;
    cur2 = cur2->next;
  }
  return 1;
}



/* TEST enqueue()
 * should produce same list as manually enqueued control list
 * return 1 on PASS
 */
int TESTenqueue(int numPackets) {

  int i;
  PacketSource_t * packetSource = createPacketSource(100, 1, 4);

  SerialList_t *queue = createSerialList();
  SerialList_t *test = createSerialList();

  int count = 0;

  for (i=0; i < numPackets; i++) {
    volatile Packet_t *packet = getUniformPacket(packetSource,0);
    if (i < 32) {
      add_list(test, i, packet);
    }
    enqueue(&count, queue, numPackets, 32, packet);
  }

  if (numPackets > 32) {
    return compList(queue, test);
  }

  return (count == DONE) && compList(queue, test);
}



/* TEST dequeue()
 * should produce same list as maually dequeued control list
 * return 1 on PASS
 */
int TESTdequeue(int n) {
  int i;
  PacketSource_t * packetSource = createPacketSource(100, 1, 4);

  SerialList_t *queue = createSerialList();
  SerialList_t *test = createSerialList();
  Item_t *curr;

  int count1 = 0, count2 = 0;
  long int f1 = 0;
  long int f2 = 0;

  for (i=0; i < n; i++) {
    volatile Packet_t *packet = getUniformPacket(packetSource,0);
    enqueue(&count1, test, n, 32, packet);
    enqueue(&count2, queue, n, 32, packet);
  }

  for (i = 0; i < n; i++) {
    dequeue(queue, &f1);
    if (test->size > 0) {
      curr = test->head;
      while (curr->next) {
	curr = curr->next;
      }
      f2 += getFingerprint((curr->value)->iterations, (curr->value)->seed);
      remove_list(test, curr->key);
      
    }
  } 
  if (!n) {
    return compList(queue, test) && (f1 == 0);
  }
  return compList(queue, test) && (f1 == f2);
}
  


/* TEST serial_queue_firewall()
 * should produce same result as Serial version
 */
int TESTserial_queue(int numPackets, int numSources, int uniformFlag, short experimentNumber) {
  long mean = 100;
  int queueDepth = 32;
  int i;

  long *check = serial_firewall(numPackets, numSources, mean, uniformFlag, experimentNumber, queueDepth); 

  long *fingerprint = serial_queue_firewall(numPackets, numSources, mean, uniformFlag, experimentNumber, queueDepth); 

  for (i = 0; i < numSources; i++) {
    if (check[i] != fingerprint[i]) {
      printf("Diffing values:\n check[%i] = %li\n squeue[%i] = %li\n", i, check[i], i, fingerprint[i]); 
      return 0;
    }
  }
  return 1;
}



int main() {

  //int i;

  printf("\nTESTS for Serial Queue (squeue_firewall.c) \nUnless stated otherwise, [n = 1, D = 32, W = 100]\n");

  res(TESTcreate(), "create_queues", "");
  res(TESTenqueue(1), "enqueue1", "(T = 1)");
  res(TESTenqueue(20), "enqueue2", "(T = 20)");
  res(TESTenqueue(21), "enqueue3", "(T = 21)");
  res(TESTenqueue(32), "enqueue4", "(T = D)");
  res(TESTenqueue(33), "enqueue5", "(T > D)");
  res(TESTdequeue(0), "dequeue1", "(T = 0)");
  res(TESTdequeue(1), "dequeue2", "(T = 1)");
  res(TESTdequeue(20), "dequeue3", "(T = 20)");
  res(TESTdequeue(32), "dequeue4", "(T = D)");
  res(TESTdequeue(33), "dequeue5", "(T > D)");

  int t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
  t1 = t2 = t3 = t4 = t5 = t6 = t7 = t8 = t9 = t10 = 1;

  // run multiple trials
  /*  for (i = 0; i < 1; i++) {
    t1 *= TESTserial_queue(1, 1, 1, i);
    t2 *= TESTserial_queue(128, 1, 1, i);
    t3 *= TESTserial_queue(128, 8, 1, i);
    t4 *= TESTserial_queue(128, 8, 0, i);
    t5 *= TESTserial_queue(128, 16, 1, i);
    }*/

  t1 = TESTserial_queue(1, 1, 1, 1);
  t2 = TESTserial_queue(1024, 1, 1, 1);
  t3 = TESTserial_queue(1024, 8, 1, 1);
  t4 = TESTserial_queue(1024, 16, 1, 1);
  t5 = TESTserial_queue(1024, 16, 0, 1);

  res(t1, "serial_queue1", "(T = 1, n = 1)");
  res(t2, "serial_queue2", "(T = 1024, n = 1)");
  res(t3, "serial_queue3", "(T = 1024, n = 8)");
  res(t5, "serial_queue4", "(T = 1024, n = 16)");
  res(t4, "serial_queue5", "(T = 1024, n = 16, exp)");

  return 0;
}
