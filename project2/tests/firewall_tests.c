#include <stdio.h>
#include <string.h>
#include "../src/funcs.h"

#define GREEN "\033[0;32m"
#define NORM "\033[0m"
#define RED "\033[0;31m"



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
    if (i < 30) {
      add_list(test, i, packet);
    }
    enqueue(&count, queue, numPackets, 30, packet);
  }

  return compList(queue, test);
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
    enqueue(&count1, test, n, 30, packet);
    enqueue(&count2, queue, n, 30, packet);
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
  


int main() {


  printf("\nTESTS for Serial Queue (squeue_firewall.c) \n\n");

  res(TESTcreate(), "create_queues", "");
  res(TESTenqueue(0), "enqueue1", "(n = 0)");
  res(TESTenqueue(1), "enqueue2", "(n = 1)");
  res(TESTenqueue(20), "enqueue3", "(n = 20)");
  res(TESTenqueue(21), "enqueue4", "(n = 21)");
  res(TESTenqueue(30), "enqueue5", "(n = queueDepth)");
  res(TESTenqueue(31), "enqueue6", "(n > queueDepth)");
  res(TESTdequeue(0), "dequeue1", "(numdq = 0)");
  res(TESTdequeue(1), "dequeue2", "(numdq = 1)");
  res(TESTdequeue(20), "dequeue3", "(numdq = 20)");
  res(TESTdequeue(30), "dequeue4", "(numdq = n)");
  res(TESTdequeue(31), "dequeue5", "(numdq > n)");
  return 0;
}
