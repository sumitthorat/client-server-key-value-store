// C program for array implementation of queue 
#ifndef DS_DEFS_H
#define DS_DEFS_H
#include <limits.h> 
#include <stdio.h> 
#include <string.h>
#include <stdlib.h>
#include <math.h>

#define HASH_NUMBER 4
#define NUM 31
#define BUCKETS 512
// A structure to represent a queue 
// struct Queue { 
// 	int front, rear, size;
//       char *req;
// 	unsigned capacity; 
// 	int* array; 
// }; 
struct QueueNode { 
      char *req;
      int clientFd;
      struct QueueNode* next; 
}; 
  
// The queue, front stores the front node of LL and rear stores the 
// last node of LL 
struct Queue { 
      int size;
      struct QueueNode *front, *rear; 
}; 
// struct QueueNode{
//       char *req;
//       int clientFd;
//       struct QueueNode *next;
// };

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
struct hash* createHashTable();
struct Queue* createQueue();
// int isFull(struct Queue* queue);
int isEmpty(struct Queue* queue);
void add(struct Queue* queue, char *req, int clientFd);
void pop(struct Queue* queue);
struct QueueNode *top(struct Queue* queue);
struct QueueNode *bottom(struct Queue* queue);
int getHashingIndex(char *key);
char *searchInHash(struct hash* hashTable,char *key);
int size(struct Queue *queue);
#endif //DS_DEFS_H