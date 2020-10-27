#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h> 
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

int SERVER_PORT;
char *SERVER_IP;
int NUM_OF_GET;
int NUM_OF_PUT;
int NUM_OF_DEL;
int NUM_OF_CLIENTS;
int NUM_OF_INTIAL_ENTRIES;

char *key_values[1000];

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
    printf("IP: %s\n",SERVER_IP);
    printf("Reading from config file...\nSERVER_PORT: %d,\nSERVER_IP: %s,\nNO_OF_GETS: %d,\nNO_OF_PUT: %d, \nNO_OF_DEL: %d\nNO_OF_CLIENTS: %d\n", SERVER_PORT,SERVER_IP,NUM_OF_GET,NUM_OF_PUT,NUM_OF_DEL,NUM_OF_CLIENTS);
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
    printf("blah: %d\n", serv_addr.sin_addr.s_addr);
    serv_addr.sin_port = htons(SERVER_PORT);
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        herror("ERROR connecting");
        fprintf(stderr, "connect() failed: %s\n", strerror(errno));
    }

    return sockfd;
}

void close_connection(int sockfd){
    close(sockfd);
}

void send_get_message(){

}

void send_del_message(){

}

void send_put_message(){

}

void populate_kv_store(int sockfd){
    unsigned char message[514];
    message[0] = '2';
    int n, i=0;
    while (i<NUM_OF_INTIAL_ENTRIES)
    {
        for (int i=1; i<513; i++)
            message[i] = 'A' + random() % 26;
        message[513] = (char)0;
        key_values[i] =  strdup(message);
        n = write(sockfd, message, strlen(message));
        if (n < 0) {
            herror("Error writing to socket");
        }

        i++;
    }
}


int main(){
    int socket;
    read_config();
    socket = connect_to_server();
    populate_kv_store(socket);
}

