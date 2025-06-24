#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h> 
#include <netinet/in.h> 
#include <strings.h>

#define BUFFER_SIZE 600
#define PORT 8080

void startQueries(int socketFd) {
    char buffer[BUFFER_SIZE];
    while(1) {
        int n = 0;
        bzero(buffer, sizeof(buffer));
        printf("cache > ");
        
        while ((buffer[n++] = getchar()) != '\n') {}

        if(strncmp(buffer, ".exit", 5) == 0) {
            printf("Exiting...\n");
            break;
        }

        send(socketFd, buffer, sizeof(buffer), 0);
        bzero(buffer, sizeof(buffer));
        read(socketFd, buffer, BUFFER_SIZE);

        printf("%s\n", buffer);
    }
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
