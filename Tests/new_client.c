#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h> 
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>

#define MSG_SIZE 9

int SERVER_PORT;
char *SERVER_IP;
int NUM_OF_GET;
int NUM_OF_PUT;
int NUM_OF_DEL;
int NUM_OF_CLIENTS;
int NUM_OF_INTIAL_ENTRIES;

pthread_mutex_t lock; 

char *key_values[1000];

unsigned long get_microsecond_timestamp(){
    struct timeval tv;
    gettimeofday(&tv,NULL);
    unsigned long time_in_micros = 1000000 * tv.tv_sec + tv.tv_usec;
    return time_in_micros;
}

void read_config(){
    FILE* fptr = fopen("client_config.txt", "r");
    size_t read, len;
    char * line = NULL;
    while ((read = getline(&line, &len, fptr)) != -1) {
        char * token = strtok(line, "=");
        
        while( token != NULL ) {
            if (strcmp(token, "SERVER_PORT")==0)
            {
                token = strtok(NULL, "=");
                SERVER_PORT = atoi(token);
            }
            else if (strcmp(token,"SERVER_IP")==0)
            {
                token = strtok(NULL, "=");
                
                SERVER_IP = strdup(token);
                
            }
            else if (strcmp(token,"NO_OF_GET")==0)
            {
                token = strtok(NULL, "=");
                NUM_OF_GET=  atoi(token);
            }
            else if (strcmp(token, "NO_OF_PUT")==0)
            {
                token = strtok(NULL, "=");
                NUM_OF_PUT=  atoi(token);
            }
            else if (strcmp(token , "NO_OF_DEL")==0)
            {
                token = strtok(NULL, "=");
                NUM_OF_DEL=  atoi(token);
            }
            else if (strcmp(token , "NO_OF_CLIENTS")==0)
            {
                token = strtok(NULL, "=");
                NUM_OF_CLIENTS=  atoi(token);
            }
            else if (strcmp(token , "NO_OF_INITIAL_ENTRIES")==0)
            {
                token = strtok(NULL, "=");
                NUM_OF_INTIAL_ENTRIES=  atoi(token);
            }
            else
            {
                token = strtok(NULL, "=");
            }  
        } 
    }
    printf("Phase 1...\n");
    printf("Reading from config file...\n");
    printf("SERVER_PORT: %d\n", SERVER_PORT);
    printf("SERVER_IP: %s", SERVER_IP);
    printf("NO_OF_GETS: %d\n", NUM_OF_GET);
    printf("NO_OF_PUTS: %d\n", NUM_OF_PUT);
    printf("NO_OF_DELS: %d\n", NUM_OF_DEL);
    printf("NO_OF_CLIENTS: %d\n", NUM_OF_CLIENTS);
    printf("NO_OF_INITIAL_ENTRIES: %d\n", NUM_OF_INTIAL_ENTRIES);
    printf("\n");
    fclose(fptr);
}


int connect_to_server(){
    int sockfd;
    struct sockaddr_in serv_addr;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd<0)
    {
        printf("No sock only\n");
    }
    
    // memset((char*)&serv_addr,0,sizeof(serv_addr));
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serv_addr.sin_port = htons(SERVER_PORT);
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        herror("ERROR connecting");
        fprintf(stderr, "connect() failed: %s\n", strerror(errno));
        exit(1);
    }

    return sockfd;
}

void close_connection(int sockfd){
    close(sockfd);
}

void send_get_message(int t_id, int sockfd){
    // struct timeval tv;
    // gettimeofday(&tv,NULL);
    // unsigned long time_in_micros = 1000000 * tv.tv_sec + tv.tv_usec;
    srand((int)get_microsecond_timestamp());
    pthread_mutex_lock(&lock);
    int index = random()%NUM_OF_INTIAL_ENTRIES;
    pthread_mutex_unlock(&lock);
    char *message = key_values[index];
    // char *key = substring(message, 0, (MSG_SIZE-1)/2 +1);
    message[0]='1';
    // key[(MSG_SIZE-1)/2 +1]='\0';
    printf("Sending a GET from thread_id: %d, for key-value: %s\n", t_id, message);
    int n = write(sockfd, message, strlen(message));
    if (n < 0) {
        herror("Error writing to socket");
    }
}

void send_del_message(int t_id, int sockfd){
    // printf("Del called by thread id: %d\n", t_id);
    // struct timeval tv;
    // gettimeofday(&tv,NULL);
    // unsigned long time_in_micros = 1000000 * tv.tv_sec + tv.tv_usec;
    srand((int)get_microsecond_timestamp());
    pthread_mutex_lock(&lock);
    int index = random()%NUM_OF_INTIAL_ENTRIES;
    NUM_OF_INTIAL_ENTRIES--;
    pthread_mutex_unlock(&lock);
    char *message = key_values[index];
    // char *key = substring(message, 0, (MSG_SIZE-1)/2 +1);
    message[0]='3';
    // key[(MSG_SIZE-1)/2 +1]='\0';
    printf("Sending a DEL from thread_id: %d, for key-value: %s\n", t_id, message);
    int n = write(sockfd, message, strlen(message));
    if (n < 0) {
        herror("Error writing to socket");
    }
}

void send_put_message(int t_id, int sockfd){
    // printf("Put called by thread id: %d\n", t_id);
    // struct timeval tv;
    // gettimeofday(&tv,NULL);
    // unsigned long time_in_micros = 1000000 * tv.tv_sec + tv.tv_usec;
    srand((int)get_microsecond_timestamp());
    pthread_mutex_lock(&lock);
    int index = random()%NUM_OF_INTIAL_ENTRIES;
    pthread_mutex_unlock(&lock);
    char *message = key_values[index];
    // char *key = substring(message, 0, (MSG_SIZE-1)/2 +1);
    // key[0]='1';
    // key[(MSG_SIZE-1)/2 +1]='\0';
    printf("Sending a PUT from thread_id: %d, for key: %s\n", t_id, message);
    int n = write(sockfd, message, strlen(message));
    if (n < 0) {
        herror("Error writing to socket");
    }
}

//Phase 1 read the config
//Phase 2 populate the KV store

// Phase 3 Get(To do the get and put, ) and put

void populate_kv_store(int sockfd){
    unsigned char message[MSG_SIZE+1];
    message[0] = '2';
    int n, i=0;
    printf("Phase 2...\n");
    printf("Populating the KV store with below messages...\n");
    while (i<NUM_OF_INTIAL_ENTRIES)
    {
        for (int i=1; i<MSG_SIZE; i++)
            message[i] = 'A' + random() % 26;
        message[MSG_SIZE] = (char)0;
        key_values[i] =  strdup(message);
        printf("Message: %s\n",message);
        
        n = write(sockfd, message, strlen(message));
        printf("Sent bytes: %d\n", n);
        if (n < 0) {
            herror("Error writing to socket");
        }

        i++;
    }
    printf("\n");
}


void *thread_fun(void *args){
    // int *t_id = (int *)args;
    int t_id = *((int *) args);
    // int id=*t_id;
    int sockfd = connect_to_server();
    int get=NUM_OF_GET;
    int put=NUM_OF_PUT;
    int del=NUM_OF_DEL;
    printf("Thread-%d initialised...\n", t_id);
    //Creating a seed (Right now not calling delete)
    while (get||put||del)
    {
        /* code */
        srand((int)get_microsecond_timestamp());
        int random_num;
        if (get && put && del)
        {
            /* code */
            random_num = rand()%3 + 1;
        }
        else if ((get && put)||(get && del)||(put && put))
        {
            /* code */
            random_num = rand()%2 + 1;
        }
        else if(get)
        {
            random_num=1;
        }else if(put){
            random_num=2;
        }else if(del){
            random_num=3;
        }
        switch(random_num) {
        case 1:
            if (get)
            {
                send_get_message(t_id, sockfd);
                get--;
            }
            break;
        case 2:
            if (put)
            {
                send_put_message(t_id, sockfd);
                put--;
            }
            break;
        case 3:
            if (del)
            {
                send_del_message(t_id, sockfd);
                del--;
            }
            break;
        default:
            return "Error: invalid option";
        }
    }
    close_connection(sockfd);
}


int main(){
    int socket;
    read_config();
    socket = connect_to_server();

    populate_kv_store(socket);
    close_connection(socket);
    //Create clients here
    if (pthread_mutex_init(&lock, NULL) != 0) { 
        printf("\n mutex init has failed\n"); 
        return 1; 
    } 
    pthread_t thread_id[NUM_OF_CLIENTS];

    printf("Phase 3...\n");
    printf("Starting the requests...\n");
    int args[NUM_OF_CLIENTS];
    for (size_t i = 0; i < NUM_OF_CLIENTS; i++)
    {
        args[i]=i+1;
        pthread_create(&thread_id[i], NULL, thread_fun, &args[i]); 
    }

    for (size_t j = 0; j < NUM_OF_CLIENTS; j++)
    {
        pthread_join(thread_id[j],NULL);
    }
    printf("\n");
        

}

