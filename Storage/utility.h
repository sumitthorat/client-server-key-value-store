#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>

#define MSG_SIZE 9
#define KEY_SIZE 4
#define VAL_SIZE 4

char *substring(char *str, int start, int end);
void error (char* msg);

// Utility function to find substring of str ie. str[start, end)
char *substring(char *str, int start, int end) {
    int bytes = (end - start + 1);
    char *substr = (char *)malloc(bytes);

    for (int i = start; i < end; i++) {
        substr[i - start] = str[i];
    }

    substr[end - start] = '\0';
    return substr;
}

// Utility function to print error message and exit
void error (char* msg) {
    perror(msg);
    exit(1);
}

unsigned long get_microsecond_timestamp(){
    struct timeval tv;
    gettimeofday(&tv,NULL);
    unsigned long time_in_micros = 1000000 * tv.tv_sec + tv.tv_usec;
    return time_in_micros;
}

#endif //UTILS_H