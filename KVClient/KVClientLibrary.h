#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <math.h>

#define RSIZE 513
#define KEY_START_IDX 1
#define VAL_START_IDX 257
#define KV_LEN 256
#define ERROR (unsigned char) 240
#define SUCCESS (unsigned char) 200

#define BILLION  1000000000.0


struct time_stats{
    double total_get_time;
    double total_put_time;
    // double average_put_time;
    // double average_get_time;
    // int total_clients;
};



int get(char* key, char** val, char** error, int serverfd);

int put(char* key, char* val, char** error, int serverfd);

int del(char* key, char** error, int serverfd);

void initialise_timer();
double get_total_get_time();
double get_total_put_time();
