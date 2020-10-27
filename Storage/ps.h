#include "utility.h"

char *find_in_PS(char *key);
void update_PS(char *key, char *val);
void remove_from_PS(char *key);

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