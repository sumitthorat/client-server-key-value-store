#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int SERVER_PORT;
long CACHE_LEN;
int NUM_WORKER_THREADS;

void read_config();

void error (char* msg) {
    perror(msg);
    exit(1);
}

int main(int argc, char** argv) {
    read_config();
    
    return 0;
}

void read_config() {
    // Open config file
    FILE* fptr = fopen("config.txt", "r");
    if (fptr == NULL) {
        error("Could not open config file");
    }

    /* 
        Read info from config file
        1. Server listening port
        2. No. of worker threads (n)
        3. Maximum entries in the cache
    */

    char buff[256];
    int buff_len = 256;
    // TODO: Handle error if required
    if (fgets(buff, buff_len, fptr) != NULL) {
        SERVER_PORT = atoi(buff);
    }

    if (fgets(buff, buff_len, fptr) != NULL) {
        NUM_WORKER_THREADS = atoi(buff);
    }

    if (fgets(buff, buff_len, fptr) != NULL) {
        CACHE_LEN = atol(buff);
    }

    fclose(fptr);
}