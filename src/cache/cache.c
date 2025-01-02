#include "cache.h"
#include <sys/socket.h>
#include <string.h>

void handleRequest(void *arg) {
    int client = *((int *) arg);
    char* buffer = (char *) malloc(BUFFER_SIZE * sizeof(char));

    ssize_t bytesRecieved = recv(
        client, //receive this client request data
        buffer, //buffer to store incoming data
        BUFFER_SIZE, 
        0
    );

    if(bytesRecieved > 0) {
        CacheAction action;

    }
}

StatementResult prepareStatement(char* buffer, CacheAction* action) {
    if(strncmp(buffer, "set", 3) == 0) {
        action = CACHE_SET;
        return PREPARE_SUCCESS;
    } else if(strncmp(buffer, "delete", 6) == 0) {
        action = CACHE_DELETE;
        return PREPARE_SUCCESS;
    }

    return PREPARE_UNRECOGNIZED_STATEMENT;
}

void executeAction(CacheAction* action) {
    switch(*action) {
        case CACHE_SET:
            printf("SET\n");
            break;
        case CACHE_DELETE:
            printf("DELETE\n");
            break;
    }
}