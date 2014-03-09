#ifndef HASHLIST_H_
#define HASHLIST_H_

#include <stdbool.h>
#include "packetsource.h"
#include "hashgenerator.h"

typedef struct HashItem {
  int key;
  volatile HashPacket_t * value;
  struct HashItem * next;
}HashItem_t;

typedef struct {
  int size;
  HashItem_t * head;
  HashItem_t * tail;
}HashList_t;

HashList_t * createHashList();

HashList_t *  createHashListWithItem(int key,  volatile HashPacket_t * value);

HashItem_t * getItem_hashlist(HashList_t * list, int key);

bool contains_hashlist(HashList_t * list, int key);

bool remove_hashlist(HashList_t * list, int key);

void add_hashlist(HashList_t * list, int key, volatile HashPacket_t * value);

void addNoCheck_hashlist(HashList_t * list, int key, volatile HashPacket_t * value);

void print_hashlist(HashList_t * list);

int hashlist_len(HashList_t *list);

#endif /* SERIALLIST_H_ */
