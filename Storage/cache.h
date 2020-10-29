#ifndef CACHE_H
#define CACHE_H
#include "ps.h"
#include "defs.h"
#define INFINITE 1<<30

extern ENTRY *cache_ptr;
extern long CACHE_LEN;

void initialize_cache();
ENTRY *find_available_cache_line();
ENTRY *LFU();
ENTRY *LRU();
ENTRY *find_in_cache(char *key);
void remove_from_cache(ENTRY *loc);
void update_cache(ENTRY *loc, char *key, char *val);

void initialize_cache() {
    cache_ptr = (ENTRY *)malloc(CACHE_LEN * sizeof(ENTRY));
    for (int i = 0; i < CACHE_LEN; i++) {
        ENTRY *ptr = cache_ptr + i;
        ptr->is_valid = 'F'; 
        //initialise lock
    }
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

void remove_from_cache(ENTRY *loc) {
    printf("remove_from_cache\n");
    loc->is_valid = 'F';
}

ENTRY *LFU() {
    printf("LFU\n");
    ENTRY *loc = find_available_cache_line();
    if (loc) {
        printf("Got a available cache line\n");
        return loc;
    }

    int min_freq = INFINITE;
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
    //Keep a data structure containing free lines (indices)
    //Update data structure
    
    printf("find_available_cache_line\n");
    for (int i = 0; i < CACHE_LEN; i++) {
        ENTRY *loc = cache_ptr + i;

        if (loc->is_valid == 'F') {
            return loc;
        }
    }

    return NULL;
}

#endif //CACHE_H