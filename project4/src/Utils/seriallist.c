#include<stdlib.h>
#include <stdio.h>
#include "seriallist.h"

/* Get list length  */
int list_len(SerialList_t *q) {
  int count = 0;
  if (!q) {
    return count;
  }
  Item_t *curr = q->head;
  while (curr) {
    count++;
    curr = curr->next;
  }
  return count;
}

SerialList_t *  createSerialList()
{
  SerialList_t * list = (SerialList_t *)malloc(sizeof(SerialList_t));
  list->size = 0;
  list->head = NULL;
  return list;
}

SerialList_t * createSerialListWithItem(int key, volatile Packet_t * value)
{ 
  SerialList_t * list = (SerialList_t *)malloc(sizeof(SerialList_t));
  list->size = 1;
  Item_t * newItem = (Item_t *)malloc(sizeof(Item_t));
  newItem->key = key;
  newItem->value = value;
  newItem->next = NULL; // ADDED
  list->head = newItem;

  return list;
}

Item_t * getItem_list(SerialList_t * list, int key){

  Item_t * curr = list->head;
  
  while(curr){
    if(curr->key == key)
      return curr;
    curr = curr->next;
  }
  return NULL;
}

bool contains_list(SerialList_t * list, int key){
  return getItem_list(list,key) != NULL;
}

bool remove_list(SerialList_t * list, int key){
  if(!contains_list(list,key))
    return false;
  
  Item_t * curr = list->head;
  
  if(curr == NULL) {
    return false;
  }
  else if (curr->key ==key){
    Item_t * temp = curr = __sync_lock_test_and_set(&list->head, list->head->next);
    __sync_fetch_and_sub(&(list->size), 1);
    free(temp);
    return true;
  }else{
    while(curr->next) {
      if(curr->next->key == key){
	Item_t * temp = __sync_lock_test_and_set(&curr->next, curr->next->next);
	__sync_fetch_and_sub(&(list->size), 1);
	free(temp);
	return true;
      }
      else
	curr = curr->next;
    }
  }
  return false;
}

void add_list(SerialList_t * list, int key, volatile Packet_t * value){
  Item_t * tmpItem = getItem_list(list,key);
  
  if(tmpItem != NULL){
    tmpItem->value = value;
  }else{
    Item_t * newItem = (Item_t *)malloc(sizeof(Item_t));
    newItem->key = key;
    newItem->value = value;
    newItem->next = list->head;
    list->head = newItem;
    __sync_fetch_and_add(&(list->size), 1);
  }
}

void addNoCheck_list(SerialList_t * list, int key, volatile Packet_t * value){

  Item_t * newItem = (Item_t *)malloc(sizeof(Item_t));
  newItem->key = key;
  newItem->value = value;
  newItem->next = list->head;
  list->head = newItem;
  __sync_fetch_and_add(&(list->size), 1);
}

void print_list(SerialList_t * list){
  Item_t * curr = list->head;
  int len = list_len(list);
  printf(" # ITEMS = %i\n", len);
  while(curr != NULL){
    printf("    addr:%p\tkey:%d\tvalue:%p\n",curr,curr->key,curr->value);
    curr = curr->next;
  }
}
