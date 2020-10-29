#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h> 
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

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

    int np = 2;
    // "np" clients
    for (int i = 0; i < np; ++i) {
        if (fork() == 0) {
            connect_send();
            exit(0);
        }
    }
    
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
    
    int pid = getpid();
    char mypid[6];   // ex. 34567
    sprintf(mypid, "%d", pid);


    int max = 2, count = 0;
    char* buff; 
    while (count < max) {
        char buff[12] = "2123";
        strcat(buff, mypid);
        printf("%s\n", buff);

        n = write(sockfd, buff, strlen(buff));
        if (n < 0) {
            error("Error writing to socket");
        }
        sleep(3);
        count++;
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