#include "utility.h"

// struct file_index{
//     int total;
//     struct file_index_node *head;
//     struct file_index_node *end;
// };

// struct file_index_node{
//     char *key;
//     int position;
//     char status;
//     struct file_index_node *next;
// };

// struct file_entry{
//     char *key;
//     char *val;
// };

// pthread_mutex_t lock;
// struct file_index *file_index;

// void initialise_file_index();
// char *find_in_PS(char *key);
// void update_PS(char *key, char *val);
// void remove_from_PS(char *key);

// void initialise_file_index(){
//     FILE *fp;
//     //printf("Creating the persistant storage...\n");
//     fp = fopen ("data.bin", "w+");
//     fclose(fp);
//     file_index = (struct file_index *)malloc(sizeof(struct file_index));
//     file_index->head = file_index->end = NULL;
//     file_index->total=0;
// }

// void update_PS(char *key, char *val){
//     FILE *fp; 
//     int pos, present=0;
//     fp = fopen ("data.bin", "r+"); 
//     struct file_entry entry = {key, val};
//     struct file_index_node *inode = file_index->head;

//     // pthread_mutex_lock(&lock);
//     while (inode)
//     {   
//         if (strcmp(inode->key, key)==0 && inode->status == 'T')
//         {
//             pos = inode->position; 
//             present = 1;
//             break;
//         }
//         inode = inode->next;
//     }
//     if (present)
//     {
//         //printf("Key found in bitmap\n");
//         fseek(fp, pos, SEEK_SET);
//         fwrite(&entry, sizeof(struct file_entry), 1, fp);
//     }
//     else
//     {
//         //printf("Key not found in bitmap(New key)\n");
//         fseek(fp, 0,SEEK_END);
//         fwrite(&entry, sizeof(struct file_entry), 1, fp);
//         file_index->total++;
//         struct file_index_node *inode =  (struct file_index_node *)malloc(sizeof(struct file_index_node));
//         inode->key = strdup(key);
//         inode->position=ftell(fp);
//         inode->status = 'T';
//         if (!file_index->head)
//         {
//             file_index->head = file_index->end = inode;
//             inode->next = NULL;
//         }
//         else
//         {
//             file_index->end->next = inode;
//             inode->next = NULL;
//             file_index->end = inode;  
//         }
//     }
//     // pthread_mutex_unlock(&lock);
//     fclose(fp);
// }

// char *find_in_PS(char *key){
//     struct file_index_node *inode = file_index->head;
//     struct file_entry entry;
//     int pos, present=0;
//     FILE *fp = fopen ("data.bin", "r"); 
//     // pthread_mutex_lock(&lock);
//     while (inode)
//     {
//         if (strcmp(inode->key, key)==0 && inode->status == 'T')
//         {
//             pos = inode->position; 
//             present = 1;
//             break;
//         }
//         inode = inode->next;
//     }
//     if(present)
//     {
//         //printf("Key found in bitmap\n");
//         fseek(fp, pos, SEEK_SET);
//         fread(&entry, sizeof(struct file_entry), 1, fp);
//     }else
//     {
//         // pthread_mutex_unlock(&lock);
//         return NULL;
//     }

//     // pthread_mutex_unlock(&lock);
//     fclose(fp);
//     return entry.val;
// }

// void remove_from_PS(char *key){
//     struct file_index_node *inode = file_index->head;
//     FILE *fp = fopen ("data.bin", "r+"); 
//     // pthread_mutex_lock(&lock);
//     while (inode)
//     {
//         if (strcmp(inode->key, key)==0 && inode->status == 'T')
//         {
//             inode->status='F';
//             break;
//         }
//         inode = inode->next;
//     }
//     // pthread_mutex_unlock(&lock);
//     fclose(fp);
//     return;
// }

// #include <stdio.h>
// #include <sys/types.h>
// #include <sys/stat.h>
// #include <fcntl.h>
// #include <string.h>
// #include <unistd.h>

#include "../RW_lock/rwlock.h"
#include <errno.h>
#define NUM_OF_FILES 67
extern int errno;

struct file_index{
    int total;
    struct file_index_node *head;
    struct file_index_node *end;
    struct rwlock rwl;
};

struct file_index_node{
    char *key;
    int position;
    char status;
    struct file_index_node *next;
};

struct file_entry{
    char *key;
    char *val;
};

struct file_index *file_index[NUM_OF_FILES];

char *get_file_name(char *filename, int number)
{
    filename[0] =(char)0;
    char file[30] ="Persistant_Store/data";
    char num[3];
    sprintf(num, "%d", number);
    strcat(file, num);
    strcat(file, ".bin");
    strcat(filename,file);
    return filename;
}

void initialise_ps()
{
    for (int i = 0; i < NUM_OF_FILES; i++)
    {
        char filename[30];
        get_file_name(filename, i);
        
        int fp = open(filename, O_CREAT, 0666);
        if( fp == -1 ) {
            fprintf(stderr, "Value of errno: %d\n", errno);
            fprintf(stderr, "Error opening file: %s\n", strerror(errno));
        } else {
            close(fp);
        }
        close(fp);
        file_index[i] = (struct file_index *)malloc(sizeof(struct file_index));
        file_index[i]->head = file_index[i]->end = NULL;
        file_index[i]->total=0;
        init_rwlock(&(file_index[i]->rwl));
    }
}

int get_file_hash_index(unsigned char *str)
{
    unsigned long hash = 5381;
    int c;
    while (c = *str++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    return hash%NUM_OF_FILES;
}

void update_PS(char *key, char *val){
    char file[30];
    int pos, present=0;
    int file_no = get_file_hash_index(key);
    get_file_name(file, file_no);
    char *filename = strdup(file);
    
    FILE *fp = fopen (filename, "r+");
    // //printf("fp -> %p\n",fp);
    struct file_entry entry = {key, val};
    
    struct file_index_node *inode = file_index[file_no]->head;

    // pthread_mutex_lock(&lock);
    write_lock(&(file_index[file_no]->rwl));
    while (inode)
    {   
        if (strcmp(inode->key, key)==0 && inode->status == 'T')
        {
            pos = inode->position; 
            present = 1;
            break;
        }
        inode = inode->next;
    }
    
    if (present)
    {
        //printf("Key found in bitmap\n");
        fseek(fp, pos, SEEK_SET);
        fwrite(&entry, sizeof(struct file_entry), 1, fp);
    }
    else
    {
        struct file_index_node *inode =  (struct file_index_node *)malloc(sizeof(struct file_index_node));
        //printf("Key not found in bitmap(New key)\n");
        fseek(fp, 0, SEEK_END);
        inode->position=ftell(fp);
        fwrite(&entry, sizeof(struct file_entry), 1, fp);
        file_index[file_no]->total++;
        inode->key = strdup(key);
        
        inode->status = 'T';
        if (!file_index[file_no]->head)
        {
            file_index[file_no]->head = file_index[file_no]->end = inode;
            inode->next = NULL;
        }
        else
        {
            file_index[file_no]->end->next = inode;
            inode->next = NULL;
            file_index[file_no]->end = inode;  
        }
    }
    // pthread_mutex_unlock(&lock);
    write_unlock(&(file_index[file_no]->rwl));
    fclose(fp);
}

char *find_in_PS(char *key){
    char filename[30];
    int file_no = get_file_hash_index(key);
    get_file_name(filename, file_no);
    struct file_index_node *inode = file_index[file_no]->head;
    struct file_entry entry;
    int pos, present=0;
    FILE *fp = fopen (filename, "r+"); 
    // pthread_mutex_lock(&lock);
    //printf("Searching in file: %s\n", filename);
    // read_lock(&(file_index[file_no]->rwl));
    while (inode)
    {
        if (strcmp(inode->key, key)==0 && inode->status == 'T')
        {
            pos = inode->position; 
            present = 1;
            break;
        }
        inode = inode->next;
    }
    if(present)
    {
        //printf("Key found in bitmap\n");
        fseek(fp, pos, SEEK_SET);
        fread(&entry, sizeof(struct file_entry), 1, fp);
    }else
    {
        // pthread_mutex_unlock(&lock);
        return NULL;
    }

    // pthread_mutex_unlock(&lock);
    // read_unlock(&(file_index[file_no]->rwl));
    fclose(fp);
    
    return entry.val;
}

void remove_from_PS(char *key){
    char filename[30];
    int file_no = get_file_hash_index(key);
    get_file_name(filename, file_no);
    struct file_index_node *inode = file_index[file_no]->head;
    FILE *fp = fopen (filename, "r+"); 
    // pthread_mutex_lock(&lock);
    write_lock(&(file_index[file_no]->rwl));
    while (inode)
    {
        if (strcmp(inode->key, key)==0 && inode->status == 'T')
        {
            inode->status='F';
            break;
        }
        inode = inode->next;
    }
    // pthread_mutex_unlock(&lock);
    write_unlock(&(file_index[file_no]->rwl));
    fclose(fp);
    return;
}