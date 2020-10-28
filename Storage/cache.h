#include "ps.h"
#include "../RW_lock/rwlock.h"

#define ENTRY struct cache_ENTRY
#define INFINITE 1<<30

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
    struct rwlock rwl;
};

// Pointer to the starting of the cache. initialize_cache() allocates appropriate memory & initializes the cache
ENTRY *cache_ptr = NULL;

// No of maximum entries in the cache  read_lock(entry->rwl)
long CACHE_LEN;

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
        init_rwlock(&(ptr->rwl));
    }
}

ENTRY *find_in_cache(char *key) {
    printf("find_in_cache\n");

    for (int i = 0; i < CACHE_LEN; i++) {
        ENTRY *loc = cache_ptr + i;
        // printf("Trying for read lock at %p\n",loc);
        read_lock(&(loc->rwl));
        // printf("Obtained read lock for %p\n",loc);
        if (loc->is_valid == 'T') {
            // printf("(cache) Found: %s\n", loc->key);
            if (strcmp(loc->key, key) == 0){
                // TODO: Move this out 
                loc->timestamp = (int)time(NULL);
                loc->freq ++;
                read_unlock(&(loc->rwl));
                // printf("Released read lock for %p\n",loc);
                return loc;
            }
        }
        read_unlock(&(loc->rwl));
        // printf("Released read lock for %p\n",loc);
    }
    
    return NULL;
}

void update_cache(ENTRY *loc, char *key, char *val) {
    printf("update_cache\n");
    // printf("Trying for write lock at %p, Read count: %d\n",loc, loc->rwl.reader_count);
    write_lock(&(loc->rwl));
    // printf("Obtained write lock for %p\n",loc);
    // loc->is_valid == 'F' --- "12-34" exists.  Put 12-56 (100)
    // 
    if (loc->is_valid == 'T' && strcmp(loc->key, key) == 0) {
        //
        loc->freq ++;
    }
    else
        loc->freq = 1;

    loc->key = key;
    loc->val = val;
    loc->is_valid = 'T';
    loc->is_dirty = 'T';
    loc->timestamp = (int)time(NULL);
    write_unlock(&(loc->rwl));
    // printf("Released write lock for %p\n",loc);
    printf("Updated entry: %s-%s(%d) \n", loc->key,loc->val, loc->freq);
}

void remove_from_cache(ENTRY *loc) {
    printf("remove_from_cache\n");
    // printf("Trying for write lock at %p, Read count: %d\n",loc, loc->rwl.reader_count);
    write_lock(&(loc->rwl));
    // printf("Obtained write lock for %p\n",loc);
    loc->is_valid = 'F';
    write_unlock(&(loc->rwl));
    // printf("Released write lock for %p\n",loc);
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
        // printf("Trying for read lock at %p\n",loc);
        read_lock(&(loc->rwl));
        // printf("Obtained read lock for %p\n",loc);
        printf("LFU: %s %d\n", loc->key, loc->freq);
        if (loc->freq < min_freq) {
            min_freq = loc->freq;
            line = loc;
        }
        read_unlock(&(loc->rwl));
        // printf("Released read lock for %p\n",loc);
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
        // printf("Trying for read lock at %p\n",loc);
        read_lock(&(loc->rwl));
        // printf("Released read lock for %p\n",loc);
        printf("LRU: %s %d\n", loc->key, loc->timestamp);
        if (loc->timestamp < oldest_time) {
            oldest_time = loc->timestamp;
            line = loc;
        }
        read_unlock(&(loc->rwl));
        // printf("Released read lock for %p\n",loc);
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
        // printf("Trying for write lock at %p, Read count: %d\n",loc, loc->rwl.reader_count);
        write_lock(&(loc->rwl));
        //Change to read lock
        // printf("Obtained write lock for %p\n",loc);
        if (loc->is_valid == 'F') {
            // printf("Hello\n");
            write_unlock(&(loc->rwl));
            // printf("Released write lock for %p\n",loc);
            return loc;
            
        }
        write_unlock(&(loc->rwl));
        // printf("Released write lock for %p\n",loc);
    }

    return NULL;
}


