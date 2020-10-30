#ifndef CACHE_H
#define CACHE_H
#include "ps.h"
#include "defs.h"
#include "../RW_lock/rwlock.h"
#include <limits.h>

#define INFINITE LONG_MAX

extern ENTRY *cache_ptr;
extern long CACHE_LEN;

// this will be used as return type from find_in_cache
struct entry_with_status {
    ENTRY *entry;
    int status;
};

void initialize_cache();
struct entry_with_status *find_in_cache(char *key);
void remove_from_cache(ENTRY *loc);
void update_cache_line(ENTRY *loc, char *key, char *val);
void update_frequency_timestamp(ENTRY *loc);

void initialize_cache() {
    cache_ptr = (ENTRY *)malloc(CACHE_LEN * sizeof(ENTRY));
    for (int i = 0; i < CACHE_LEN; i++) {
        ENTRY *ptr = cache_ptr + i;
        ptr->is_valid = 'F'; 
        init_rwlock(&(ptr->rwl));
        printf("Initialised lock at %p\n", ptr);
    }
    printf("Size of entry: %ld\n", sizeof(ENTRY));
}

/*
    This function returns the cache line entry along with a status
    Status = 1 -> key is found and corresponding cache line is returned
    Status = 2 -> an available Cache line is returned
    Status = 3 -> Cache line with LRU Key is returned
*/
struct entry_with_status *find_in_cache(char *key) {
    printf("find_in_cache\n");

    int status = 3; //by default status = 3
    unsigned long oldest_timestamp = LONG_MAX;
    ENTRY *entry = NULL;
    for (int i = 0; i < CACHE_LEN; i++) {
        ENTRY *loc = cache_ptr + i;
        
        read_lock(&(loc->rwl));
        // printf("Obtained read lock for %p\n",loc);
        if (loc->is_valid == 'T') {
            // printf("(cache) Found: %s\n", loc->key);
            if (strcmp(loc->key, key) == 0) {
                // printf("Released read lock for %p\n",loc);
                read_unlock(&(loc->rwl));
                entry = loc;
                status = 1;
                break;
            }
        } else if (loc->is_valid == 'F') {
            printf("At loc %p\n",loc);
            printf("Status update to 2\n");
            
            status = 2;
            entry = loc;
        }
        // if we have already got an available cache line, don't try for LRU  1604035667498196
        if ((status != 2 && status != 1) && loc->is_valid == 'T' && oldest_timestamp > loc->timestamp) {
            printf("At loc %p\n",loc);
            printf("Updating LRU block\n");
            oldest_timestamp = (long int)loc->timestamp;
            entry = loc;
        }

        read_unlock(&(loc->rwl));
        // printf("Released read lock for %p\n",loc);
    }
    struct entry_with_status *ret = (struct entry_with_status *)malloc(sizeof(struct entry_with_status));
    ret->entry = entry;
    ret->status = status;
    return ret;
}

void update_cache_line(ENTRY *loc, char *key, char *val) {
    printf("update_cache_line\n");
    
    if (loc->is_valid == 'T' && strcmp(loc->key, key) == 0)
        loc->freq ++;
    else
        loc->freq = 1;

    loc->key = key;
    loc->val = val;
    loc->is_valid = 'T';
    loc->is_dirty = 'T';
    loc->timestamp = get_microsecond_timestamp();
    printf("Updated entry at loc %p: %s-%s(%d) \n", loc, loc->key,loc->val, loc->freq);
}

void remove_from_cache(ENTRY *loc) {
    printf("remove_from_cache\n");
    loc->is_valid = 'F';
    loc->freq = 0;
}

void update_frequency_timestamp(ENTRY *loc){
    loc->freq++;
    loc->timestamp = get_microsecond_timestamp();
}

#endif //CACHE_H