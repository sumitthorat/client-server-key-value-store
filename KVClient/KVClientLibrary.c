#include "KVClientLibrary.h"

double total_get_time=0;
double total_put_time=0;
pthread_mutex_t get_time_lock, put_time_lock, del_time_lock;

struct time_stats *ts;

struct time_stats* initialise_timer(){
    ts = (struct time_stats *)malloc(sizeof(struct time_stats));
    ts->total_get_time = 0;
    ts->total_put_time = 0;
    ts->total_del_time = 0;
    pthread_mutex_init(&get_time_lock, NULL);
    pthread_mutex_init(&put_time_lock, NULL);
    pthread_mutex_init(&del_time_lock, NULL);
    return ts;
}

void destroy_timer(struct time_stats* ts){
    free(ts);
}

double get_total_get_time(){
    return ts->total_get_time;
}

double get_total_put_time(){
    return ts->total_put_time;
}

double get_total_del_time(){
    return ts->total_del_time;
}


// Makes 's' a string of length KV_LEN, where the last byte (index = KV_LEN) is \0
char* add_padding(char *s) {
    char* ret = (char*) malloc(sizeof(char) * (KV_LEN + 1));
    strcpy(ret, s);

    for (int i = strlen(s); i < KV_LEN; i++) {
            ret[i] = '.';
    }

    ret[KV_LEN] = '\0';

    return ret;
}

void remove_padding(char *s) {
    for (int i = 0; i < KV_LEN; i++) {
        if (s[i] == '.') {
            s[i] = '\0';
            return;
        }
    }
}

int get(char* key, char** val, char** error, int serverfd) {
    // printf("Inside get api\n");

    if( !key ) {
        *error = (char*) malloc(sizeof(char) * (KV_LEN + 1));
        strncpy(*error, "Error: Memory not allocated to key", KV_LEN);
        return -1;
    }
    else if (strlen(key) > KV_LEN)
    {
        *error = (char*) malloc(sizeof(char) * (KV_LEN + 1));
        strncpy(*error, "Error: Key size is greater than 256B", KV_LEN);
        return -1;
    }
    // Form the request
    struct timespec start, end;
    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    // setsockopt(serverfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
    // printf("Before add padding\n");
    char* padded_key = add_padding(key);
    // printf("After add padding key %s, len = %d\n", padded_key, strlen(padded_key));
    char* padded_val = add_padding("\0"); // It will generate a length 256 bytes string
    // printf("After add padding val %s, len = %d\n", padded_val, strlen(padded_val));

    char req[RSIZE + 1];
    req[RSIZE] = '\0';
    req[0] = '1';
    strncpy(req + KEY_START_IDX, padded_key, KV_LEN);
    strncpy(req + VAL_START_IDX, padded_val, KV_LEN);
    // printf("Req = %s, len = %ld\n", req, strlen(req));

    // printf("Req: %s\n", req);
    
    // Send request
    clock_gettime(CLOCK_REALTIME, &start);
    int writen = write(serverfd, req, RSIZE);

    // Wait for response
    char* resp = (char*) malloc(sizeof(char) * (RSIZE + 1));
    ssize_t readn = read(serverfd, resp, RSIZE);
    resp[RSIZE] = '\0';


    // printf("Resp = %s, len = %ld\n", resp, strlen(resp));

    clock_gettime(CLOCK_REALTIME, &end);
    double time_spent;
    if ((end.tv_nsec-start.tv_nsec)<0)
    {
        struct timespec temp;
        temp.tv_sec = end.tv_sec-start.tv_sec-1;
        temp.tv_nsec = BILLION+end.tv_nsec-start.tv_nsec;
        time_spent = temp.tv_sec + temp.tv_nsec / BILLION;
    }
    else
    {
        time_spent = ((end.tv_sec - start.tv_sec) +(end.tv_nsec - start.tv_nsec)) / BILLION;
    }
    // printf("Before Free\n");
    pthread_mutex_lock(&get_time_lock);
    ts->total_get_time+=time_spent;
    pthread_mutex_unlock(&get_time_lock);
    // printf("Response time for GET is %f micro-seconds\n", time_spent*pow(10,6));
    
    free(padded_key);
    free(padded_val);

    // printf("After free\n");

    // Malloc val/error memory
    if (readn <= 0 || (unsigned char) resp[0] == ERROR) {
        // printf("Before malloc\n");
        *error = (char*) malloc(sizeof(char) * (KV_LEN + 1));
        // printf("Before strnvpy\n");
        strncpy(*error, resp + VAL_START_IDX, KV_LEN);
        // printf("After strnvpy\n");
        (*error)[KV_LEN] = '\0';
        remove_padding(*error);
        printf("Error: %s\n", *error);
        // printf("Leaving get api err\n");
        return -1;
    }

    *val = (char*) malloc(sizeof(char) * (KV_LEN + 1));
    // printf("Before stncpy\n");
    // printf("Val from GET API: %s\n", resp + VAL_START_IDX);
    strncpy(*val, resp + VAL_START_IDX, KV_LEN);
    // printf("Before KVLEN\n");
    (*val)[KV_LEN] = '\0';
    // printf("Before removing pad\n");
    remove_padding(*val);
    // printf("Val from GET API: %s %d\n", *val, (unsigned char)resp[0]);
    // printf("Leaving get api\n");
    return 0;
}


int put(char* key, char* val, char** error, int serverfd) {
    // printf("Inside put api\n");

    if(!key) {
        *error = (char*) malloc(sizeof(char) * (KV_LEN + 1));
        strncpy(*error, "Error: Memory not allocated to key", KV_LEN);
        return -1;
    } 
    else if(strlen(key) > KV_LEN)
    {
        // printf("Key = %s, len = %ld\n", key, strlen(key));
        *error = (char*) malloc(sizeof(char) * (KV_LEN + 1));
        strncpy(*error, "Error: Key size is greater than 256B", KV_LEN);
        return -1;
    }
    if(!val){
        *error = (char*) malloc(sizeof(char) * (KV_LEN + 1));
        strncpy(*error, "Error: Memory not allocated to val", KV_LEN);
        return -1;
    }
    else if(strlen(val) > KV_LEN)
    {
        *error = (char*) malloc(sizeof(char) * (KV_LEN + 1));
        strncpy(*error, "Error: Value size is greater than 256B", KV_LEN);
        return -1;
    }


    

    char* padded_key = add_padding(key);
    char* padded_val = add_padding(val);


    // printf("After padding: key %ld val %ld\n", strlen(key), strlen(val));
    
    // struct timeval tv;
    // tv.tv_sec = 5;
    // tv.tv_usec = 0;
    // setsockopt(serverfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

    char req[RSIZE + 1];
    req[0] = '2';
    req[RSIZE] = '\0';
    strncpy(req + KEY_START_IDX, padded_key, KV_LEN);
    strncpy(req + VAL_START_IDX, padded_val, KV_LEN);

    // printf("PUT REQ Being sent = %s, len = %ld\n", req, strlen(req));
    struct timespec start, end;
    // Send request
    clock_gettime(CLOCK_REALTIME, &start);
    size_t writen = write(serverfd, req, RSIZE);

    // Wait for response
    char* resp = (char*) malloc(sizeof(char) * (RSIZE + 1));
    size_t readn = read(serverfd, resp, RSIZE);
    resp[RSIZE] = '\0';


    // printf("PUT RESP = %s, len = %ld\n", resp, strlen(resp));

    clock_gettime(CLOCK_REALTIME, &end);
    double time_spent;
    if ((end.tv_nsec-start.tv_nsec)<0)
    {
        struct timespec temp;
        temp.tv_sec = end.tv_sec-start.tv_sec-1;
        temp.tv_nsec = BILLION+end.tv_nsec-start.tv_nsec;
        time_spent = temp.tv_sec + temp.tv_nsec / BILLION;
    }
    else
    {
        time_spent = ((end.tv_sec - start.tv_sec) +(end.tv_nsec - start.tv_nsec)) / BILLION;
    }
    
    // printf("Before lock\n");   
    pthread_mutex_lock(&put_time_lock);
    ts->total_put_time+=time_spent;
    pthread_mutex_unlock(&put_time_lock);
    // printf("After lock\n");   
	// printf("Response time for PUT is %f micro-seconds \n", time_spent*pow(10,6));

    // printf("After remove_padding %ld %ld %s\n", strlen(key), strlen(val), resp);
    // Malloc error memory
    free(padded_key);
    free(padded_val);
    // printf("Response: %s\n", resp);
    // printf("PUT Resp: %s\n", resp);
    if (readn <= 0) {
        // printf("Leaving put api, readn <= 0\n");
        return -1;
    }

    if ((unsigned char) resp[0] == ERROR) {
        // printf("Error: %s\n", *error);
        *error = (char*) malloc(sizeof(char) * (KV_LEN + 1));
        strncpy(*error, resp + VAL_START_IDX, KV_LEN);
        (*error)[KV_LEN] = '\0';
        remove_padding(*error);
        // printf("Leaving put api err\n");
        return -1;
    }

    // printf("Leaving put api\n");
    return 0;
}


int del(char* key, char** error, int serverfd) {

    // printf("Inside del api\n");

    if( !key ){
        *error = (char*) malloc(sizeof(char) * (KV_LEN + 1));
        strncpy(*error, "Error: Memory not allocated to key", KV_LEN);
        return -1;
    }
    else if( strlen(key) > KV_LEN)
    {
        *error = (char*) malloc(sizeof(char) * (KV_LEN + 1));
        strncpy(*error, "Error: Key size is greater than 256B", KV_LEN);
        return -1;
    }

    char* padded_key = add_padding(key);
    char* padded_val = add_padding("\0");

    char req[RSIZE + 1];
    req[0] = '3';
    req[RSIZE] = '\0';
    strncpy(req + KEY_START_IDX, padded_key, KV_LEN);
    strncpy(req + VAL_START_IDX, padded_val, KV_LEN);

    // Send request
    struct timespec start, end;
    clock_gettime(CLOCK_REALTIME, &start);
    int writen = write(serverfd, req, RSIZE);

    // Wait for response
    char* resp = (char*) malloc(sizeof(char) * (RSIZE + 1));
    int readn = read(serverfd, resp, RSIZE);
    resp[RSIZE] = '\0';

    // printf("DEL RESP = %s, len = %d\n", resp, strlen(resp));
    // printf("Resp = %s, len = %ld\n", resp, strlen(resp));

    clock_gettime(CLOCK_REALTIME, &end);
    double time_spent;
    if ((end.tv_nsec-start.tv_nsec)<0)
    {
        struct timespec temp;
        temp.tv_sec = end.tv_sec-start.tv_sec-1;
        temp.tv_nsec = BILLION+end.tv_nsec-start.tv_nsec;
        time_spent = temp.tv_sec + temp.tv_nsec / BILLION;
    }
    else
    {
        time_spent = ((end.tv_sec - start.tv_sec) +(end.tv_nsec - start.tv_nsec)) / BILLION;
    }

    pthread_mutex_lock(&del_time_lock);
    ts->total_del_time+=time_spent;
    pthread_mutex_unlock(&del_time_lock);

    free(padded_key);
    free(padded_val);

    // Malloc error memory
    if (readn <= 0 || (unsigned char) resp[0] == ERROR) {
        *error = (char*) malloc(sizeof(char) * (KV_LEN + 1));
        strncpy(*error, resp + VAL_START_IDX, KV_LEN);
        (*error)[KV_LEN] = '\0';
        remove_padding(*error);
        // printf("Leaving del api err\n");
        return -1;
    }

    // printf("Leaving del api\n");
    return 0;
}


