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

// #define MSG_SIZE 513
// #define KEY_SIZE 256
// #define VAL_SIZE 256
#define ENTRY struct cache_ENTRY
#define INFINITY 1<<30
#define MSG_SIZE 9
#define KEY_SIZE 4
#define VAL_SIZE 4

/*
    Structure of each cache entry
    key: The key
    val: value corresponding to the key
    is_valid: 'T' if key is present in the cache entry, otherwise 'F'  
    is_dirty: 'T' if the value is modified in the cache but not updated in the Persistent Storage, otherwise 'F'
    freq: No of accesses of the key since the key is added to the cache
    timestamp: Timestamp when the key was accessed last time
*/
struct cache_ENTRY {
    char *key;
    char *val;
    char is_valid;
    char is_dirty;
    int freq;
    int timestamp;
};

// Pointer to the starting of the cache. initialize_cache() allocates appropriate memory & initializes the cache
ENTRY *cache_ptr = NULL;

// No of maximum entries in the cache 
long CACHE_LEN;

void initialize_cache();
char *handle_requests(char *msg);
char *get(char *msg);
void put(char *msg);
void del(char *msg);
ENTRY *find_available_cache_line();
ENTRY *LFU();
ENTRY *LRU();
ENTRY *find_in_cache(char *key);
void remove_from_cache(ENTRY *loc);
void update_cache(ENTRY *loc, char *key, char *val);
char *find_in_PS(char *key);
void update_PS(char *key, char *val);
void remove_from_PS(char *key);

void error (char* msg);

// Utility function to find substring of str
char *substring(char *str, int start, int end) {
    int bytes = (end - start + 1);
    char *substr = (char *)malloc(bytes);

    for (int i = start; i < end; i++) {
        substr[i - start] = str[i];
    }

    return substr;
}

// Utility function to print error message and exit
void error (char* msg) {
    perror(msg);
    exit(1);
}

void initialize_cache() {
    cache_ptr = (ENTRY *)malloc(CACHE_LEN * sizeof(ENTRY));
    for (int i = 0; i < CACHE_LEN; i++) {
        ENTRY *ptr = cache_ptr + i;
        ptr->is_valid = 'F'; 
    }
}

// handles all the request
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

// GET request handler
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

// PUT request handler
void put(char *msg) {
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
}

// DEL request handler
void del(char *msg) {
    printf("del\n");
    char *key =  substring(msg, 0, KEY_SIZE);

    ENTRY *entry = find_in_cache(key);

    if (entry)
        remove_from_cache(entry);
        
    remove_from_PS(key);

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
        error("Could not open PS.txt");
        
    // 1 byte for storing whether it is delete/insert entry
    // 1 byte for storing NULL character at the end    
    int buf_len = KEY_SIZE + VAL_SIZE + 1 + 1;
    char *buf = (char *)malloc(buf_len); 
    
    // start reading from the last key-val pair
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
        
        // move backward and read key-val pairs
        int offset = lseek(fd, -2 * (KEY_SIZE + VAL_SIZE + 1), SEEK_CUR);
        if (offset < 0)
            break;
    }
    free(buf);
    close(fd);
    return NULL;
}

void update_cache(ENTRY *loc, char *key, char *val) {
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
}

void update_PS(char *key, char *val) {
    printf("update_PS\n");
    int fd = open("PS.txt", O_WRONLY | O_APPEND | O_CREAT, 0666);
    if (fd < 0)
        error("Could not open PS.txt");
        
    // 1 for storing whether it is delete/insert entry
    int buf_len = KEY_SIZE + VAL_SIZE + 1;
    char *buf = (char *)malloc(buf_len); 
    
    int index = 0;
    buf[index++] = 'I'; // it is insert entry in the log
    for (int i = 0; i < KEY_SIZE; i++)
        buf[index++] = key[i];
    
    for (int i = 0; i < VAL_SIZE; i++)
        buf[index++] = val[i];

    write(fd, buf, buf_len);
    free(buf);    
    close(fd);
}

void remove_from_cache(ENTRY *loc) {
    printf("remove_from_cache\n");
    loc->is_valid = 'F';
}

void remove_from_PS(char *key) {
    printf("remove_from_PS\n");
    int fd = open("PS.txt", O_WRONLY | O_APPEND | O_CREAT, 0666);
    if (fd < 0)
        error("Could not open PS.txt");
        
    // 1 for storing whether it is delete/insert entry
    int buf_len = KEY_SIZE + VAL_SIZE + 1;
    char *buf = (char *)malloc(buf_len); 
    
    int index = 0;
    buf[index++] = 'D'; // it is delete entry in the log
    for (int i = 0; i < KEY_SIZE; i++)
        buf[index++] = key[i];
    
    // store garbage val

    write(fd, buf, buf_len);
    free(buf);    
    close(fd);
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
