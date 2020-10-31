#include "ds_defs.h"

  struct node * createNode(char *key) {
        struct node *newnode;
        newnode = (struct node *)malloc(sizeof(struct node));
        newnode->key = strdup(key);
        newnode->next = NULL;
        return newnode;
  }

  void insertToHash(struct hash* hashTable, char *key) {
        int sum=0;
        for (size_t i = 0; i < HASH_NUMBER; i++)
        {
            int temp = (key[i] & NUM)-1;
            sum += temp;      
        }
        int hashIndex = sum % (HASH_NUMBER*26);
        struct node *newnode =  createNode(key);
        
        /* head of list for the bucket with index "hashIndex" */
        if (!hashTable[hashIndex].head) {
                hashTable[hashIndex].head = newnode;
                hashTable[hashIndex].count = 1;
                return;
        }
        /* adding new node to the list */
        newnode->next = (hashTable[hashIndex].head);
        /*
         * update the head of the list and no of
         * nodes in the current bucket
         */
        hashTable[hashIndex].head = newnode;
        hashTable[hashIndex].count++;
        return;
  }

  void deleteFromHash(struct hash* hashTable,char *key) {
        /* find the bucket using hash index */
       int sum=0, flag = 0;;
        for (size_t i = 0; i < HASH_NUMBER; i++)
        {
            int temp = (key[i] & NUM)-1;
            sum += temp;      
        }
      
        int hashIndex = sum % (HASH_NUMBER*26);
        
        struct node *temp, *myNode;
        /* get the list head from current bucket */
        myNode = hashTable[hashIndex].head;
        if (!myNode) {
                // printf("Given data is not present in hash Table!!\n");
                return;
        }
        temp = myNode;
        while (myNode != NULL) {
                /* delete the node with given key */
                if (strcmp(myNode->key ,key)==0) {
                        if (myNode == hashTable[hashIndex].head)
                                hashTable[hashIndex].head = myNode->next;
                        else
                                temp->next = myNode->next;
                        hashTable[hashIndex].count--;
                        free(myNode);
                        break;
                }
                temp = myNode;
                myNode = myNode->next;
        }
        // if (flag)
        //         printf("Data deleted successfully from Hash Table\n");
        // else
        //         printf("Given data is not present in hash Table!!!!\n");
        // return;
  }

  char *searchInHash(struct hash* hashTable,char *key) {
        int sum=0,flag;
        for (size_t i = 0; i < HASH_NUMBER; i++)
        {
            int temp = (key[i] & NUM)-1;
            sum += temp;      
        }
        int hashIndex = sum % (HASH_NUMBER*26);
        struct node *myNode;
        myNode = hashTable[hashIndex].head;
        if (!myNode) {
                return NULL;
        }
        while (myNode != NULL) {
                if (strcmp(myNode->key, key)==0) {
                        return myNode->key;
                }
                myNode = myNode->next;
        }
        // if (!flag)
        //         printf("Search element unavailable in hash table\n");
        return NULL;
  }

  struct hash* createHashTable(unsigned buckets) 
{ 
	struct hash* hashTable = (struct hash*)malloc(buckets*sizeof(struct hash)); 
	return hashTable; 
} 

//   void display(struct hash* hashTable) {
//         struct node *myNode;
//         int i;
//         for (i = 0; i < HASH_NUMBER*4; i++) {
//                 if (hashTable[i].count == 0)
//                         continue;
//                 myNode = hashTable[i].head;
//                 if (!myNode)
//                         continue;
//                 printf("\nData at index %d in Hash Table with head %p:\n", i, hashTable[i].head);
//                 printf("Key    \n");
//                 printf("--------------------------------\n");
//                 while (myNode != NULL) {
//                         printf("%s \t%p\t %p\n", myNode->key,myNode, myNode->next);
//                         myNode = myNode->next;
//                 }
//         }
//         return;
//   }

// int main(){
//     struct hash *table = createHashTable(HASH_NUMBER*26);
//     printf("Loc %p\n",table);
//     insertToHash(table, "CCCA");
//     insertToHash(table, "ABCD");
//     insertToHash(table, "DDAA");
//     display(table);
//     // printf("At postion 6: %d\n", table[6].count);
//     deleteFromHash(table, "ABCD");
//     display(table);
//     // printf("At postion 6: %d\n", table[6].count);
//     // printf("Key: %s\n",searchInHash(table, "DDAA"));
//     deleteFromHash(table, "DDAA");
//     display(table);
//     // printf("At postion 6: %d\n", table[6].count);
//     // printf("Key: %s\n",searchInHash(table, "DDAA"));
//     // printf("Key: %s\n",searchInHash(table, "ABCD"));
// }