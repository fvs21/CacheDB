#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h> 
#include <netinet/in.h> 
#include <strings.h>

#define BUFFER_SIZE 10490
#define PORT 8080

void startQueries(int socketFd) {
    char* buffer = (char *) malloc(BUFFER_SIZE);

    for(;;) {
        size_t allocated = 0;

        bzero(buffer, BUFFER_SIZE);
        printf("cache > ");
        
        ssize_t bytes_read = getline(&buffer, &allocated, stdin);
        if(bytes_read <= 0) {
            perror("Error reading input");
            exit(0);
        }
        buffer[bytes_read - 1] = 0;

        if(strncmp(buffer, ".exit", 5) == 0) {
            printf("Exiting...\n");
            break;
        }

        write(socketFd, buffer, bytes_read);
        bzero(buffer, BUFFER_SIZE);
        recv(socketFd, buffer, BUFFER_SIZE, 0);
        printf("%s\n", buffer);
    }
    free(buffer);
}

int main() {
    int socketfd;
    struct sockaddr_in serverAddress;

    socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if(socketfd == -1) {
        perror("Failed to initialize connection.");
        exit(0);
    }

    printf("Connecting...\n");

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1"); //server address
    serverAddress.sin_port = htons(PORT);

    if(connect(socketfd, (struct sockaddr*) &serverAddress, sizeof(serverAddress)) != 0) {
       perror("Failed to connect to the server");
       exit(0);
    }

    printf("Connection established\n");

    startQueries(socketfd);

    close(socketfd);
}