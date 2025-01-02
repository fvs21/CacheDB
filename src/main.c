#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h> 
#include <sys/types.h> 
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include "cache.h"

#define PORT 8080

int main(int argc, char** argv) {	
	int server;
	struct sockaddr_in serverAddress;

	printf("Initializing server...\n");

	server = socket(
		AF_INET, //use ipv4
		SOCK_STREAM, //use TCP
		0 //use IP
	);

	if(server == -1) {
		perror("Error initializing server");
		exit(0);
	}

	serverAddress.sin_family = AF_INET; //use ipv4
	serverAddress.sin_addr.s_addr = INADDR_ANY; //accept connections to all IPs
	serverAddress.sin_port = htons(PORT);

	//binds the socket to the chose port
	if(bind(server, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
		perror("Error initializing server. Bind failed.");
		exit(0);
	}

	//start listening for connections and take the maximum number of pending connections
	if(listen(server, 10) != 0) {
		perror("Server failed to listen.");
		exit(0);
	}

	printf("Server initialized.\n");

	for(;;) {
		struct sockaddr_in clientAddress;
		socklen_t client_addr_len = sizeof(clientAddress);
		int *client = malloc(sizeof(int)); //incoming client

		//accept a client connection
		if((*client = accept(server, (struct sockaddr *)&clientAddress, &client_addr_len)) < 0) {
			perror("Failed to accept an incomming client connection.");
			continue;
		}

		pthread_t threadId;
		pthread_create(
			&threadId, //variable where the proccess id will be stored
			NULL, //default thread attributes
			handleRequest, //function to run
			(void *)client //function parameters
		);
		pthread_detach(threadId);
	}
}