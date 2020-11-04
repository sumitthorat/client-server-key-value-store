#include "KVClientLibrary.h"


int get(char* key, char** val, char** error, int serverfd) {
    // Form the request
    char req[RSIZE];
    req[0] = '1';
    strncpy(req + KEY_START_IDX, key, KV_LEN);
    bzero(req + VAL_START_IDX, KV_LEN);
    
    // Send request
    int writen = write(serverfd, req, RSIZE);

    // Wait for response
    char* resp = (char*) malloc(sizeof(char) * RSIZE);
    int readn = read(serverfd, resp, RSIZE);
    

    // Malloc val/error memory
    if (readn <= 0 || resp[0] == ERROR) {
        *error = (char*) malloc(sizeof(char) * KV_LEN);
        strncpy(*error, resp + VAL_START_IDX, KV_LEN);
        return -1;
    }

    *val = (char*) malloc(sizeof(char) * KV_LEN);
    strncpy(*val, resp + VAL_START_IDX, KV_LEN);

    return 0;
}


int put(char* key, char* val, char** error, int serverfd) {
    char req[RSIZE];
    req[0] = '2';

    strncpy(req + KEY_START_IDX, key, KV_LEN);
    strncpy(req + VAL_START_IDX, val, KV_LEN);

    // Send request
    size_t writen = write(serverfd, req, RSIZE);

    // Wait for response
    char* resp = (char*) malloc(sizeof(char) * RSIZE);
    size_t readn = read(serverfd, resp, RSIZE);
    

    // Malloc error memory
    if (readn <= 0) {
        return -1;
    }

    if (resp[0] == ERROR) {
        *error = (char*) malloc(sizeof(char) * KV_LEN);
        strncpy(*error, resp + VAL_START_IDX, KV_LEN);
        return -1;
    }
    return 0;
}


int del(char* key, char** error, int serverfd) {
    char req[RSIZE];
    req[0] = '3';
    strncpy(req + KEY_START_IDX, key, KV_LEN);

    // Send request
    int writen = write(serverfd, req, RSIZE);

    // Wait for response
    char* resp = (char*) malloc(sizeof(char) * RSIZE);
    int readn = read(serverfd, resp, RSIZE);

    // Malloc error memory
    if (readn <= 0 || resp[0] == ERROR) {
        *error = (char*) malloc(sizeof(char) * KV_LEN);
        strncpy(*error, resp + VAL_START_IDX, KV_LEN);
        return -1;
    }

    return 0;
}


