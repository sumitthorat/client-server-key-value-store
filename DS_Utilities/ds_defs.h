// C program for array implementation of queue 
#ifndef DS_DEFS_H
#define DS_DEFS_H
#include <limits.h> 
#include <stdio.h> 
#include <string.h>
#include <stdlib.h>

#define HASH_NUMBER 4
#define NUM 31

// A structure to represent a queue 
struct Queue { 
	int front, rear, size; 
	unsigned capacity; 
	int* array; 
}; 
struct node {
        char *key;
        struct node *next;
  };

  struct hash {
        struct node *head;
        int count;
  };

struct node * createNode(char *key);
void insertToHash(struct hash* hashTable, char *key);
void deleteFromHash(struct hash* hashTable,char *key);
struct hash* createHashTable(unsigned buckets);
struct Queue* createQueue(unsigned capacity);
int isFull(struct Queue* queue);
int isEmpty(struct Queue* queue);
void enqueue(struct Queue* queue, int item);
int dequeue(struct Queue* queue);
int front(struct Queue* queue);
int rear(struct Queue* queue);
#endif //DS_DEFS_H