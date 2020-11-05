#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define RSIZE 9
#define KEY_START_IDX 1
#define VAL_START_IDX 5
#define KV_LEN 4
#define ERROR (char) 240
#define SUCCESS (char) 200

int get(char* key, char** val, char** error, int serverfd);

int put(char* key, char* val, char** error, int serverfd);

int del(char* key, char** error, int serverfd);


