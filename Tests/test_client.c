#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h> 
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define MSG_SIZE 9
#define KEY_SIZE 4
#define VAL_SIZE 4

int SERVER_PORT;
long CACHE_LEN;
int NUM_WORKER_THREADS;

void connect_send();
void read_config();
void error(char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char** argv) {
    // Read the config file
    read_config();

    connect_send();
           
    return 0;
}

void connect_send() {
    int sockfd, n;
    struct sockaddr_in serv_addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        error("ERROR opening socket");
    }
        
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serv_addr.sin_port = htons(SERVER_PORT);

    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        error("ERROR connecting");
    }
    
    char* buff; 
    while (1) {
        char key[KEY_SIZE], val[VAL_SIZE];
        char buff[MSG_SIZE + 1];
        int status;
        printf("Enter op status, Key & val\n");
        scanf("%d %s %s", &status, key, val);
        
        sprintf(buff, "%c%s%s", '0' + status, key, val);
        buff[MSG_SIZE] = '\0';

        n = write(sockfd, buff, MSG_SIZE);
        printf("Sent Msg: %s\n", buff);
        if (n < 0) {
            error("Error writing to socket");
        }
    }

    close(sockfd);

    printf("Done...\n");
}

void read_config() {
    // Open config file
    FILE* fptr = fopen("config.txt", "r");
    if (fptr == NULL) {
        error("Could not open config file");
    }

    /* 
        Read info from config file
        1. Server listening port
        2. No. of worker threads (n)
        3. Maximum entries in the cache
    */

    char buff[256];
    int buff_len = 256;
    // TODO: Handle error if required
    if (fgets(buff, buff_len, fptr) != NULL) {
        SERVER_PORT = atoi(buff);
    }

    if (fgets(buff, buff_len, fptr) != NULL) {
        NUM_WORKER_THREADS = atoi(buff);
    }

    if (fgets(buff, buff_len, fptr) != NULL) {
        CACHE_LEN = atol(buff);
    }

    fclose(fptr);
}