#include "../Storage/cache.h"


#define MSG_SIZE 9
#define KEY_SIZE 4
#define VAL_SIZE 4



char *handle_requests(char *msg);
char *get(char *msg);
void put(char *msg);
void del(char *msg);


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
        // Updating the timestamp & frequency after a get for the key.
        entry->timestamp = (int)time(NULL);
        entry->freq ++;
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
