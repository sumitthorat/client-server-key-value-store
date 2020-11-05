#include "utility.h"

struct file_index{
    int total;
    struct file_index_node *head;
    struct file_index_node *end;
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

pthread_mutex_t lock;
struct file_index *file_index;

void initialise_file_index();
char *find_in_PS(char *key);
void update_PS(char *key, char *val);
void remove_from_PS(char *key);

void initialise_file_index(){
    FILE *fp;
    printf("Creating the persistant storage...\n");
    fp = fopen ("data.bin", "w+");
    fclose(fp);
    file_index = (struct file_index *)malloc(sizeof(struct file_index));
    file_index->head = file_index->end = NULL;
    file_index->total=0;
}

void update_PS(char *key, char *val){
    FILE *fp; 
    int pos, present=0;
    fp = fopen ("data.bin", "r+"); 
    struct file_entry entry = {key, val};
    struct file_index_node *inode = file_index->head;

    // pthread_mutex_lock(&lock);
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
        printf("Key found in bitmap\n");
        fseek(fp, pos, SEEK_SET);
        fwrite(&entry, sizeof(struct file_entry), 1, fp);
    }
    else
    {
        printf("Key not found in bitmap(New key)\n");
        fseek(fp, 0,SEEK_END);
        fwrite(&entry, sizeof(struct file_entry), 1, fp);
        file_index->total++;
        struct file_index_node *inode =  (struct file_index_node *)malloc(sizeof(struct file_index_node));
        inode->key = strdup(key);
        inode->position=ftell(fp);
        inode->status = 'T';
        if (!file_index->head)
        {
            file_index->head = file_index->end = inode;
            inode->next = NULL;
        }
        else
        {
            file_index->end->next = inode;
            inode->next = NULL;
            file_index->end = inode;  
        }
    }
    // pthread_mutex_unlock(&lock);
    fclose(fp);
}

char *find_in_PS(char *key){
    struct file_index_node *inode = file_index->head;
    struct file_entry entry;
    int pos, present=0;
    FILE *fp = fopen ("data.bin", "r"); 
    // pthread_mutex_lock(&lock);
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
        printf("Key found in bitmap\n");
        fseek(fp, pos, SEEK_SET);
        fread(&entry, sizeof(struct file_entry), 1, fp);
    }else
    {
        // pthread_mutex_unlock(&lock);
        return NULL;
    }

    // pthread_mutex_unlock(&lock);
    fclose(fp);
    return entry.val;
}

void remove_from_PS(char *key){
    struct file_index_node *inode = file_index->head;
    FILE *fp = fopen ("data.bin", "r+"); 
    // pthread_mutex_lock(&lock);
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
    fclose(fp);
    return;
}