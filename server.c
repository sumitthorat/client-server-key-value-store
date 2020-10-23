#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>

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

int main(int argc, char** argv) {
    read_config();
    initialize_cache();

    // Test - 1
    // put("1111haha");

    // printf("\n\n");

    // put("2222RKRK");

    // printf("\n\n");

    // printf("\n\n");

    // put("3333rrrr");

    // printf("%s\n", get("1111aaaa"));

    // printf("\n\n");

    // printf("%s\n", get("2222aaaa"));

    // printf("\n\n");

    // printf("%s\n", get("3333aaaa"));

    // del("3333aaaa");
    // printf("\n\n");

    // printf("%s\n", get("3333aaaa"));

    put("1111aaaa");
    put("1111bbbb");
    put("1111cccc");
    put("1111dddd");
    put("2222bbbb");
    put("3333dddd");
    printf("\n\n");

    printf("%s\n", get("1111aaaa"));

    printf("\n\n");

    printf("%s\n", get("2222aaaa"));

    printf("\n\n");

    printf("%s\n", get("3333aaaa"));

    // while(1) {
    //     char str[1 + KEY_SIZE + VAL_SIZE];
    //     printf("Enter\n");
    //     scanf("%s",str);
    //     char *res = handle_requests(str);
    //     if (res)
    //         printf("RES: %s\n", res);
    // }
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
    if (entry)
        return entry->val;
    else {
        char *val = find_in_PS(key);

        // key is not present in the PS
        if(!val) 
            return "Error: key not present";
        else
            return val; //TODO: add ENTRY to the cache
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