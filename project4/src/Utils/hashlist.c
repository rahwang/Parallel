#include<stdlib.h>
#include <stdio.h>
#include "hashlist.h"

HashList_t *  createHashList()
{
	HashList_t * list = (HashList_t *)malloc(sizeof(HashList_t));
	list->size = 0;
	list->head = NULL;
	list->tail = NULL;
	return list;
}

HashItem_t * getItem_hashlist(HashList_t * list, int key){
  HashItem_t *curr = list->head;
  
  while(curr != NULL){
    if(curr->key == key)
      return curr;
    curr = curr->next;
  }
  return NULL;
}

bool contains_hashlist(HashList_t * list, int key){
  return getItem_hashlist(list,key) != NULL;
}

bool remove_hashlist(HashList_t * list, int key){
  if(!contains_hashlist(list,key))
    return false;
  
  HashItem_t * curr = list->head;
  
  if(curr == NULL)
    return false;
  else if (curr->key ==key){
    HashItem_t * temp = curr;
    if (curr->next != NULL) {
      list->head = list->head->next;
    }
    else {
      list->head = NULL;
    }
    free(temp);
    list->size--;
    return true;
  }else{
    while(curr->next != NULL) {
      if(curr->next->key == key){
	HashItem_t * temp = curr->next;
	curr->next = curr->next->next;
	if (curr->next == NULL) {
	  list->tail = curr;
	}
	free(temp);
	list->size--;
	return true;
      }
      else
	curr = curr->next;
    }
  }
  return false;
}

void add_hashlist(HashList_t * list, int key, volatile HashPacket_t * value){
  HashItem_t * tmpItem = getItem_hashlist(list,key);
  
  if(tmpItem != NULL){
    tmpItem->value = value;
  }else{
    HashItem_t * newItem = (HashItem_t *)malloc(sizeof(HashItem_t));
    newItem->key = key;
    newItem->value = value;
    newItem->next = list->head;
    list->head = newItem;
    if (list->size == 0) {
      list->tail = newItem;
    }
    list->size++;
  }
}
void addNoCheck_hashlist(HashList_t * list, int key, volatile HashPacket_t * value){

  HashItem_t * newItem = (HashItem_t *)malloc(sizeof(HashItem_t));
  newItem->key = key;
  newItem->value = value;
  newItem->next = list->head;
  list->head = newItem;
  list->size++;
}

void print_hashlist(HashList_t * list){
  HashItem_t * curr = list->head;
  
  while(curr != NULL){
    printf("addr:%p\tkey:%d\tvalue:%p\n",curr,curr->key,curr->value);
    curr = curr->next;
  }
}
