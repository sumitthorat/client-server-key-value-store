#ifndef DEFS_H
#define DEFS_H
#include <stdio.h>
#define ENTRY struct cache_ENTRY

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
    // struct rwlock rwl;
};

// Pointer to the starting of the cache. initialize_cache() allocates appropriate memory & initializes the cache
ENTRY *cache_ptr = NULL;

// No of maximum entries in the cache  read_lock(entry->rwl)
long CACHE_LEN;
#endif //DEFS_H