#include "../Storage/cache.h"


#define MSG_SIZE 9
#define KEY_SIZE 4
#define VAL_SIZE 4

#define SUCCESS_CODE 200
#define ERROR_CODE 240

#define SET_MSG(s, code, a, b) sprintf(s, "%c%s%s", code, a, b)



void handle_requests(char *msg, char*);
void get(char *msg, char*);
void put(char *msg, char*);
void del(char *msg, char*);


void handle_requests(char *msg, char* resp) {
    char *key = substring(msg, 0, KEY_SIZE);

    if (strlen(msg) > MSG_SIZE) {
        SET_MSG(resp, ERROR_CODE, key, "Erro"); 
        return;
    }

    char status_code = *msg;

    switch(status_code) {
        case '1':
            get(msg + 1, resp);
            break;
        case '2':
            put(msg + 1, resp);
            break;
        case '3':
            del(msg + 1, resp);
            break;
        default:
            SET_MSG(resp, ERROR_CODE, key, "Erro"); 
    }
}

// GET request handler
void get(char *msg, char* resp) {
    printf("get\n");

    char *key = substring(msg, 0, KEY_SIZE);
<<<<<<< HEAD
    key[KEY_SIZE]=(char)0;
    printf("Key: %s\n", key);
    struct entry_with_status *entry_with_status_val = find_in_cache(key);
=======
    struct entry_with_status *entry_with_status_val = find_update_cache_line(key, NULL, 1);
>>>>>>> master
    ENTRY *loc = entry_with_status_val->entry;
    int status = entry_with_status_val->status;
    free(entry_with_status_val);

    // key is present in the cache
    if (status == 1) {
        printf("Got value =\"%s\"\n", loc->val);
<<<<<<< HEAD
        write_lock(&(loc->rwl));
        if (strcmp(loc->key, key) == 0) //why are we comparing again within status 1?
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
=======
        
        // sprintf(resp, "4%s%s", key, loc->val); 
        SET_MSG(resp, SUCCESS_CODE, key, loc->val);
>>>>>>> master

        free(key);

    } else {
        printf("Entry not present in cache, searching the PS\n");
        char *val = find_in_PS(key);
        

        // key is not present in the PS
        if(!val) {
            printf("Error: key not present\n");
            // return "Error: key not present";
            SET_MSG(resp, ERROR_CODE, key, "Erro"); 
        }
        else {
            printf("Got value =\"%s\"\n", val);
            SET_MSG(resp, SUCCESS_CODE, key, val);  
            // return val; //TODO: add ENTRY to the cache
        }

        free(key);
    }
}

// PUT request handler
void put(char *msg, char* resp) {
    printf("put\n");
    char *key = substring(msg, 0, KEY_SIZE);
    char *val = substring(msg, KEY_SIZE, KEY_SIZE + VAL_SIZE);
<<<<<<< HEAD
    printf("Key: %s, value: %s\n", key,val);
    struct entry_with_status *entry_with_status_val = find_in_cache(key);
    ENTRY *loc = entry_with_status_val->entry;
    int status = entry_with_status_val->status;
    free(entry_with_status_val);
    printf("Status : %d\n", status);
=======

    struct entry_with_status *entry_with_status_val = find_update_cache_line(key, val, 2);
    ENTRY *loc = entry_with_status_val->entry;
    int status = entry_with_status_val->status;
    free(entry_with_status_val);

    if (status == 1) {
        // key-val is updated in cache line
        SET_MSG(resp, SUCCESS_CODE, key, val);
        return;
    }
        
>>>>>>> master
    int flag = 0;
    char *backup_key;
    char *backup_val;
    write_lock(&(loc->rwl));
    // key is present in the cache
    // if (status == 2 || status == 3) {
    //     if (loc->is_valid == 'T' && loc->is_dirty == 'T') {
    //         backup_key = loc->key;
    //         backup_val = loc->val;
    //         flag = 1;
    //     }

    //     // Since we will do lazy update, currenly we don't care whether it is present in PS or not
    //     update_cache_line(loc, key, val); 
    // }
    
    // if (flag) {
    //     update_PS(backup_key, backup_val); 
    // }
    if (status==1 || status == 2)
    {
        update_cache_line(loc, key, val);
    }
    else if(status == 3)
    {
        backup_key = loc->key;
        backup_val = loc->val;
        update_cache_line(loc, key, val);
        update_PS(backup_key, backup_val);
    }
    
    
    write_unlock(&(loc->rwl)); //TODO: can we move this before update_PS ?

    SET_MSG(resp, SUCCESS_CODE, key, val);
}

// DEL request handler
void del(char *msg, char* resp) {
    printf("del\n");
    // TODO: SEGMENTATION FAULT even with SINGLE CLIENT AND DEL
    char *key =  substring(msg, 0, KEY_SIZE);

    struct entry_with_status *entry_with_status_val = find_update_cache_line(key, NULL, 3);

    // printf("Entry %s\n", entry_with_status_val->entry->val ? entry_with_status_val->entry->val : "NULL");

    // TODO: Change the second param to success/error
    SET_MSG(resp, SUCCESS_CODE, key, "NULL"); 
    
    free(entry_with_status_val);
    remove_from_PS(key);
    free(key);
}
