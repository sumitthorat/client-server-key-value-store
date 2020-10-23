#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>

#include "RW_lock/rwlock.h"

// #define MSG_SIZE 513
// #define KEY_SIZE 256
// #define VAL_SIZE 256
#define ENTRY struct cache_ENTRY
#define INFINITY 1<<30
#define MSG_SIZE 9
#define KEY_SIZE 4
#define VAL_SIZE 4

struct cache_ENTRY {
    char *key;
    char *val;
    char is_valid;
    char is_dirty;
    int freq;
    int timestamp;
};

ENTRY *cache_ptr = NULL;

int SERVER_PORT;
long CACHE_LEN;
int NUM_WORKER_THREADS;
int* worker_epoll_fds;
int sockfd;

void read_config();
void initialize_cache();

char *handle_requests(char *msg);
char *get(char *msg);
char *put(char *msg);
char *del(char *msg);

ENTRY *find_available_cache_line();
ENTRY *LFU();
ENTRY *LRU();

ENTRY *find_in_cache(char *key);
char *remove_from_cache(ENTRY *loc);
char *update_cache(ENTRY *loc, char *key, char *val);

char *find_in_PS(char *key);
char *update_PS(char *key, char *val);
char *remove_from_PS(char *key);

void* worker(void*);

// Utility function to find substring of str
char *substring(char *str, int start, int end) {
    int bytes = (end - start + 1);
    char *substr = (char *)malloc(bytes);

    for (int i = start; i < end; i++) {
        substr[i - start] = str[i];
    }

    return substr;
}

void error (char* msg) {
    perror(msg);
    exit(1);
}

void signal_handler(int sig) {
    close(sockfd);
    exit(0);
}

int main(int argc, char** argv) {
    // Register signal handler
    signal(SIGINT, signal_handler); // for ctrl-c

    // Read config file
    read_config();
    initialize_cache();
    
    struct sockaddr_in serv_addr, cli_addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        error("Error in server listening socket");
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERVER_PORT);
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        error("Error in binding");
    }

    listen(sockfd, 5);

    /*
        Setup the data structures for each worker thread i.e. epoll instance specific to each thread
        epoll_create for n threads, add to array
    */
    worker_epoll_fds = (int*) malloc(sizeof(int) * NUM_WORKER_THREADS);
    int t_ids[NUM_WORKER_THREADS];
    pthread_t ids[NUM_WORKER_THREADS];
    for (int i = 0; i < NUM_WORKER_THREADS; ++i) {
        // Create epoll instance for each thread 
        worker_epoll_fds[i] = epoll_create1(0);
        if (worker_epoll_fds[i] == -1) {
            error("Failed to create epoll instance");
        }

        // Create and spawn worker thread
		t_ids[i] = i;
		pthread_create(&ids[i], NULL, worker, (void*) &t_ids[i]);
	
    }


    // Conitnuously accept new connectionsd
    int wt = 0;
    struct epoll_event ev;
    while (1) {
        // Accept incoming connection
        int clilen = sizeof(cli_addr);
        ev.data.fd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        printf("Accepted new connection: %d, assigned to WT = %d\n", cli_addr.sin_port, wt);
        
        // Set event listener and assign to appropriate worker thread 
        ev.events = EPOLLIN;
        if (epoll_ctl(worker_epoll_fds[wt], EPOLL_CTL_ADD, ev.data.fd, &ev) < 0) {
            error("Error in epoll adding");
        }

        wt = ++wt % NUM_WORKER_THREADS;
    }



    for (int i = 0; i < NUM_WORKER_THREADS; ++i) {
        pthread_join(ids[i], NULL);
        close(worker_epoll_fds[i]);
    }
    
    return 0;
}

void* worker(void* arg) {
    int id = *((int*)arg);

    printf("Thread %d is ready\n", id);


    // // Probe the file descriptors
    struct epoll_event events[8];
    
    while (1) {
        // printf("New round\n");
        int nfds = epoll_wait(worker_epoll_fds[id], events, 8, 10000);

        char buff[11];
        int buff_len = 11;
        for (int i = 0; i < nfds; ++i) {
            memset(buff, 0, buff_len);
            ssize_t len = read(events[i].data.fd, buff, buff_len);
            if (len == 0) {
                close(events[i].data.fd);
                continue;
            }

            if (len < 0) {
                error("Read Error");
            }

            
            printf("WT = %d, MSG = %s\n", id, buff);
            handle_requests(buff);
            // Here request will be parsed and appropriate action will be taken
        }
    }
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

void initialize_cache() {
    cache_ptr = malloc(CACHE_LEN * sizeof(ENTRY));
    for (int i = 0; i < CACHE_LEN; i++) {
        ENTRY *ptr = cache_ptr + i;
        ptr->is_valid = 'F'; 
    }
}

char *handle_requests(char *msg) {
    if (strlen(msg) > MSG_SIZE)
        return "Error: msg size execeeded";

    char status_code = *msg;
    switch(status_code) {
        case '1':
            get(msg + 1);
            break;
        case '2':
            put(msg + 1);
            break;
        case '3':
            del(msg + 1);
            break;
        default:
            return "Error: invalid option";
    }
}

char *get(char *msg) {
    printf("get\n");
    char *key = substring(msg, 0, KEY_SIZE);
    ENTRY *entry = find_in_cache(key);
    
    // key is present in the cache
    if (entry) {
        printf("Got value =\"%s\"\n", entry->val);
        return entry->val;
    }
    else {
        char *val = find_in_PS(key);

        // key is not present in the PS
        if(!val) {
            printf("Error: key not present\n");
            return "Error: key not present";
        }
        else {
            printf("Got value =\"%s\"\n", val);
            return val; //TODO: add ENTRY to the cache
        }
    }

    free(key);
}

char *put(char *msg) {
    printf("put\n");
    char *key = substring(msg, 0, KEY_SIZE);
    char *val = substring(msg, KEY_SIZE, KEY_SIZE + VAL_SIZE);

    ENTRY *entry = find_in_cache(key);
    // key is present in the cache
    if (entry)
        update_cache(entry, key, val);
    else {
        ENTRY *loc = LRU();
        update_cache(loc, key, val); // Since we will do lazy update, currenly we don't care whether it is present in PS or not
    }

    // free(key);
    // free(val);
}

char *del(char *msg) {
    printf("del\n");
    char *key =  substring(msg, 0, KEY_SIZE);

    ENTRY *entry = find_in_cache(key);

    if (entry)
        remove_from_cache(entry);
        
    return remove_from_PS(key);

    free(key);
}

ENTRY *find_in_cache(char *key) {
    printf("find_in_cache\n");

    for (int i = 0; i < CACHE_LEN; i++) {
        ENTRY *loc = cache_ptr + i;
        if (loc->is_valid == 'T') {
            printf("Found: %s\n", loc->key);
            if (strcmp(loc->key, key) == 0) 
                return loc;
        }
    }
    
    return NULL;
}

char *find_in_PS(char *key) {
    printf("find_in_PS\n");
    int fd = open("PS.txt", O_RDONLY, 0666);
    if (fd < 0)
        perror("Could not open PS.txt");
        
    // 1 byte for storing whether it is delete/insert entry
    // 1 byte for storing NULL character at the end    
    int buf_len = KEY_SIZE + VAL_SIZE + 1 + 1;
    char *buf = (char *)malloc(buf_len); 
    
    lseek(fd, - (KEY_SIZE + VAL_SIZE + 1), SEEK_END);

    while(1) {
        int x = read(fd, buf, buf_len - 1);
        buf[x] = '\0';
        char *thiskey = substring(buf, 1, KEY_SIZE + 1);

        if (strcmp(thiskey, key) == 0) {
            free(thiskey);
            if (buf[0] == 'D') // Delete entry present in log
                return NULL;
            else // Insert entry present in log
                return buf + 1 + KEY_SIZE;
        }
        
        int offset = lseek(fd, -2 * (KEY_SIZE + VAL_SIZE + 1), SEEK_CUR);
        if (offset < 0)
            break;
    }
    free(buf);
    close(fd);
}

char *update_cache(ENTRY *loc, char *key, char *val) {
    printf("update_cache\n");
    if (loc->is_valid == 'T' && strcmp(loc->key, key) == 0) {
        loc->freq ++;
    }
    else
        loc->freq = 1;

    loc->key = key;
    loc->val = val;
    loc->is_valid = 'T';
    loc->is_dirty = 'T';
    loc->timestamp = (int)time(NULL);
    printf("Updated entry: %s (%d) \n", loc->key, loc->freq);
    return NULL;
}

char *update_PS(char *key, char *val) {
    printf("update_PS\n");
    int fd = open("PS.txt", O_WRONLY | O_APPEND | O_CREAT, 0666);
    if (fd < 0)
        error("Could not open PS.txt");
        
    // 1 for storing whether it is delete/insert entry
    int buf_len = KEY_SIZE + VAL_SIZE + 1;
    char *buf = (char *)malloc(buf_len); 
    
    int index = 0;
    buf[index++] = 'I'; // insert entry
    for (int i = 0; i < KEY_SIZE; i++)
        buf[index++] = key[i];
    
    for (int i = 0; i < VAL_SIZE; i++)
        buf[index++] = val[i];

    write(fd, buf, buf_len);
    free(buf);    
    close(fd);
    return NULL;
}

char *remove_from_cache(ENTRY *loc) {
    printf("remove_from_cache\n");
    loc->is_valid = 'F';
    return NULL;
}

char *remove_from_PS(char *key) {
    printf("remove_from_PS\n");
    int fd = open("PS.txt", O_WRONLY | O_APPEND | O_CREAT, 0666);
    if (fd < 0)
        error("Could not open PS.txt");
        
    // 1 for storing whether it is delete/insert entry
    int buf_len = KEY_SIZE + VAL_SIZE + 1;
    char *buf = (char *)malloc(buf_len); 
    
    int index = 0;
    buf[index++] = 'D'; // insert entry
    for (int i = 0; i < KEY_SIZE; i++)
        buf[index++] = key[i];
    
    // store garbage val

    write(fd, buf, buf_len);
    free(buf);    
    close(fd);
    return NULL;   
}

ENTRY *LFU() {
    printf("LFU\n");
    ENTRY *loc = find_available_cache_line();
    if (loc) {
        printf("Got a available cache line\n");
        return loc;
    }

    int min_freq = INFINITY;
    ENTRY *line = NULL;
    for (int i = 0; i < CACHE_LEN; i++) {
        loc = cache_ptr + i;
        printf("LFU: %s %d\n", loc->key, loc->freq);
        if (loc->freq < min_freq) {
            min_freq = loc->freq;
            line = loc;
        }
    }

    printf("LFU selected %s %d\n", line->key, line->freq);
    if (line->is_valid == 'T' && line->is_dirty == 'T') // there is some dirty ENTRY, push that in PS
        update_PS(line->key, line->val);

    remove_from_cache(line); 
    return line;
}

ENTRY *LRU() {
    printf("LRU\n");
    ENTRY *loc = find_available_cache_line();
    if (loc) {
        printf("Got a available cache line\n");
        return loc;
    }
    
    int oldest_time = (int)time(NULL) + 1;
    ENTRY *line = NULL;
    for (int i = 0; i < CACHE_LEN; i++) {
        loc = cache_ptr + i;

        printf("LRU: %s %d\n", loc->key, loc->freq);
        if (loc->timestamp < oldest_time) {
            oldest_time = loc->timestamp;
            line = loc;
        }
    }

    printf("LRU selected %s %d\n", line->key, line->freq);
    if (line->is_valid == 'T' && line->is_dirty == 'T') // there is some dirty ENTRY, push that in PS
        update_PS(line->key, line->val);

    remove_from_cache(line); 
    return line;
}

ENTRY *find_available_cache_line() {
    printf("find_available_cache_line\n");
    for (int i = 0; i < CACHE_LEN; i++) {
        ENTRY *loc = cache_ptr + i;

        if (loc->is_valid == 'F') {
            return loc;
        }
    }

    return NULL;
}
