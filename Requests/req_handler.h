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
        return "Error: msg size exceeded";

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
    struct entry_with_status *entry_with_status_val = find_in_cache(key);
    ENTRY *loc = entry_with_status_val->entry;
    int status = entry_with_status_val->status;
    free(entry_with_status_val);

    // key is present in the cache
    if (status == 1) {
        int exit_if = 0;
        printf("Got value =\"%s\"\n", loc->val);
        write_lock(&(loc->rwl));
        if (strcmp(loc->key, key) == 0)
            update_frequency_timestamp(loc); // Updating the timestamp & frequency after a get for the key.
        else
            exit_if = 1;

        write_unlock(&(loc->rwl));

        if (!exit_if) {
            free(key);
            return loc->val;
        }    
    }
    
    printf("Entry not present in cache, searching the PS\n");
    char *val = find_in_PS(key);
    free(key);

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

// PUT request handler
void put(char *msg) {
    printf("put\n");
    char *key = substring(msg, 0, KEY_SIZE);
    char *val = substring(msg, KEY_SIZE, KEY_SIZE + VAL_SIZE);

    struct entry_with_status *entry_with_status_val = find_in_cache(key);
    ENTRY *loc = entry_with_status_val->entry;
    int status = entry_with_status_val->status;
    free(entry_with_status_val);

    int flag = 0;
    char *backup_key;
    char *backup_val;
    write_lock(&(loc->rwl));
    // key is present in the cache
    if (status == 2 || status == 3) {
        if (loc->is_valid == 'T' && loc->is_dirty == 'T') {
            backup_key = loc->key;
            backup_val = loc->val;
            flag = 1;
        }

        // Since we will do lazy update, currenly we don't care whether it is present in PS or not
        update_cache_line(loc, key, val); 
    }
    
    if (flag) {
        update_PS(backup_key, backup_val); 
    }

    write_unlock(&(loc->rwl)); //TODO: can we move this before update_PS ?
}

// DEL request handler
void del(char *msg) {
    printf("del\n");
    char *key =  substring(msg, 0, KEY_SIZE);

    struct entry_with_status *entry_with_status_val = find_in_cache(key);
    ENTRY *loc = entry_with_status_val->entry;
    int status = entry_with_status_val->status;
    free(entry_with_status_val);

    if (status == 1) {
        write_lock(&(loc->rwl));
        if (strcmp(loc->key, key) == 0)
            remove_from_cache(loc);

        write_unlock(&(loc->rwl));
    }
        
    remove_from_PS(key);
    free(key);
}
