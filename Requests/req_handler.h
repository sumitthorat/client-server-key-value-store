#include "../Storage/cache.h"


#define MSG_SIZE 513
#define KEY_SIZE 256
#define VAL_SIZE 256

#define SUCCESS_CODE 200
#define ERROR_CODE 240

#define SET_MSG(s, code, a, b) add_padding(s, code, a, b)

void handle_requests(char *msg, char*, int id);
void get(char *msg, char*, int id);
void put(char *msg, char*, int id);
void del(char *msg, char*, int id);

void add_padding(char *s, int code, char *a, char *b) {
    sprintf(s, "%c%s%s", code, a, b);

    for (int i = 1 + strlen(a) + strlen(b); i < MSG_SIZE; i++) {
            s[i] = '.';
    }
}

void handle_requests(char *msg, char* resp, int id) {
    char *key = substring(msg, 1, KEY_SIZE + 1);
    // msg = substring(msg, 0, MSG_SIZE + 1);
    if (strlen(msg) > MSG_SIZE) {
        SET_MSG(resp, ERROR_CODE, key, "ERROR: MSG SIZE IS MORE THAN 513 BYTES");
        // printf("handle req: set msg %s %ld\n", resp, strlen(msg)); 
        return;
    } else if (strlen(msg) < MSG_SIZE) {
        SET_MSG(resp, ERROR_CODE, key, "ERROR: MSG SIZE IS LESS THAN 513 BYTES");
        // printf("handle req: set msg %s %ld\n", resp, strlen(msg)); 
        return;
    }

    char status_code = *msg;

    switch(status_code) {
        case '1':
            get(msg + 1, resp, id);
            break;
        case '2':
            put(msg + 1, resp, id);
            break;
        case '3':
            del(msg + 1, resp, id);
            break;
        default:
            SET_MSG(resp, ERROR_CODE, key, "Error: Invalid request"); 
    }
}

// GET request handler
void get(char *msg, char* resp, int id) {
    //printf("get\n");

    char *key = substring(msg, 0, KEY_SIZE);
    struct entry_with_status *entry_with_status_val = find_update_cache_line(key, NULL, 1, id);
    ENTRY *loc = entry_with_status_val->entry;
    int status = entry_with_status_val->status;
    free(entry_with_status_val);

    // key is present in the cache
    if (status == 1) {
        //printf("Got value =\"%s\"\n", loc->val);
        
        sprintf(resp, "4%s%s", key, loc->val); 
        SET_MSG(resp, SUCCESS_CODE, key, loc->val);

        free(key);

    } else {
        //printf("Entry not present in cache, searching the PS\n");
        char *val = find_in_PS(key);
        

        // key is not present in the PS
        if(!val) {
            //printf("Error: key not present\n");
            // return "Error: key not present";
            SET_MSG(resp, ERROR_CODE, key, "ERROR: GET key not found"); 
        }
        else {

            // Get the key-val in cache
            // write_lock(&(loc->rwl));
            // char *backup_key, *backup_val;
            // int flag = 0;
            // if (loc->is_valid == 'T' && loc->is_dirty == 'T') {
            //     backup_key = loc->key;
            //     backup_val = loc->val;
            //     flag = 1;
            // }

            // // Since we will do lazy update, currenly we don't care whether it is present in PS or not
            // update_cache_line(loc, key, val); 
            // if (flag) {
            //     update_PS(backup_key, backup_val); 
            // }
            // write_unlock(&(loc->rwl));
            //printf("Got value =\"%s\"\n", val);
            SET_MSG(resp, SUCCESS_CODE, key, val);  
            // return val;
        }

        free(key);
    }
}

// PUT request handler
void put(char *msg, char* resp, int id) {
    //printf("put\n");
    char *key = substring(msg, 0, KEY_SIZE);
    char *val = substring(msg, KEY_SIZE, KEY_SIZE + VAL_SIZE);

    struct entry_with_status *entry_with_status_val = find_update_cache_line(key, val, 2, id);
    ENTRY *loc = entry_with_status_val->entry;
    int status = entry_with_status_val->status;
    free(entry_with_status_val);

    if (status == 1) {
        // key-val is updated in cache line
        SET_MSG(resp, SUCCESS_CODE, key, val);
        return;
    }
        
    int flag = 0;
    char *backup_key;
    char *backup_val;
    int i = loc-cache_ptr;
    //printf("Trying for write lock at %d with reader count: %d\n",i, loc->rwl.reader_count);
    write_lock(&(loc->rwl));
    //printf("Obtained write lock \n");
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
    // if (status==1 || status == 2)
    // {
    //     update_cache_line(loc, key, val);
    // }
    // else if(status == 3)
    // {
    //     backup_key = loc->key;
    //     backup_val = loc->val;
    //     update_cache_line(loc, key, val);
    //     update_PS(backup_key, backup_val);
    // }
    
    
    write_unlock(&(loc->rwl));
    //printf("Unlocked write lock at %d\n", i);
    SET_MSG(resp, SUCCESS_CODE, key, val);
}

// DEL request handler
void del(char *msg, char* resp, int id) {
    //printf("del\n");
    char *key =  substring(msg, 0, KEY_SIZE);
    struct entry_with_status *entry_with_status_val = find_update_cache_line(key, NULL, 3, id);
    ENTRY *loc = entry_with_status_val->entry;
    int status = entry_with_status_val->status;
    // //printf("Entry %s\n", entry_with_status_val->entry->val ? entry_with_status_val->entry->val : "NULL");    
    
    free(entry_with_status_val);
    int code = remove_from_PS(key);

    if (code == 1 || status == 1) 
        SET_MSG(resp, SUCCESS_CODE, key, "Success: Key deleted"); 
    else 
        SET_MSG(resp, ERROR_CODE, key, "Error: DEL key not found"); 

    free(key);
}
