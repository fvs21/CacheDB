#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include "cache.h"

void *handleRequest(void *arg) {
    int client = *((int *) arg);
    char buffer[BUFFER_SIZE];

    ssize_t bytesRecieved = recv(
        client, //receive this client request data
        &buffer, //buffer to store incoming data
        BUFFER_SIZE, 
        0
    );

    if(bytesRecieved > 0) {
        Statement statement;
        StatementResult statementResult = prepareStatement(buffer, &statement);

        switch(statementResult) {
            case(PREPARE_SUCCESS):
                break;
            case(PREPARE_SYNTAX_ERROR):
                printf("Syntax error.\n");
            case(PREPARE_UNRECOGNIZED_STATEMENT):
                printf("Unrecognized keyword.\n");
        }

        ExecuteResult result;

        switch(statement.action) {
            case(CACHE_SET):
                result = executeSet(&statement, table);
                break;
            case(CACHE_DELETE): 
                result = executeDelete(&statement, table);
                break;
            case(CACHE_GET):
                result = executeGet(&statement, table, client);
                break;
        }

        switch(result) {
            case(EXECUTE_SUCCESS):
                sendDataToClient(client, "Success");
                break;
            case(EXECUTE_CACHE_FULL):
                sendDataToClient(client, "Cache full");
                break;
            case(EXECUTE_MEMORY_ERROR):
                sendDataToClient(client, "Memory error");
                break;
        }
    }

    //free(buffer);
    free(arg);
    close(client);
    return NULL;
}

StatementResult prepareStatement(char* buffer, Statement* statement) {
    if(strncmp(buffer, "set", 3) == 0) {
        statement->action = CACHE_SET; //set action to set

        float cacheDuration;
        TableData *key_value_to_insert = (TableData *) &(statement->data); //cast statement data to TableData

        int args_assigned = sscanf(
            buffer,
            "set %s %s %f", //set "key" "value" "cache_duration" (in milliseconds)
            (key_value_to_insert->key),
            (key_value_to_insert->value),
            &cacheDuration
        );

        if(args_assigned < 3 || cacheDuration < 0) {
            return PREPARE_SYNTAX_ERROR;
        }

        struct tm expiration = calculateCacheExpirationDate(cacheDuration);
        key_value_to_insert->expiration = expiration;

        return PREPARE_SUCCESS;
    } else if(strncmp(buffer, "delete", 6) == 0) {
        statement->action = CACHE_DELETE;

        char *key = ((char *) &(statement->data));

        int args_assigned = sscanf(
            buffer,
            "get %s",
            key
        );

        if(args_assigned < 1) {
            return PREPARE_SYNTAX_ERROR;
        }
        return PREPARE_SUCCESS;
    } else if(strncmp(buffer, "get", 3) == 0) {
        statement->action = CACHE_GET;
        
        char *key = ((char *) &(statement->data));

        int args_assigned = sscanf(
            buffer,
            "get %s",
            key
        );

        if(args_assigned < 1) {
            return PREPARE_SYNTAX_ERROR;
        }
        return PREPARE_SUCCESS;
    }
    return PREPARE_UNRECOGNIZED_STATEMENT;
}

struct tm calculateCacheExpirationDate(int milliseconds) {
    time_t now = time(NULL);

    struct tm now_tm = *localtime(&now);

    now_tm.tm_sec += (milliseconds / 1000);

    return now_tm;
}

unsigned int hash(const char* key) {
    unsigned long hash = 5381;
    for(int c = 0; c<strlen(key); c++) {
        hash = ((hash << 5) + hash) + tolower(*key);
    }

    return hash % BUCKETS;
}

ExecuteResult executeSet(Statement* statement, Bucket* table) {
    TableData key_value_to_insert = *((TableData *) &(statement->data));
    int hashIndex = hash(key_value_to_insert.key);

    Bucket* bucket = &table[hashIndex];
    Node* head = bucket->head; //wow

    if(head == NULL) {
        Node* node = malloc(sizeof(Node));
        if(node == NULL) {
            free(node);
            return EXECUTE_MEMORY_ERROR;
        }
        node->tableData = key_value_to_insert;
        node->next = NULL;
        bucket->head = node; //wow
        bucket->numOfElements = 1;
    } else {
        if(bucket->numOfElements == BUCKET_MAX_ELEMS) {
            return EXECUTE_CACHE_FULL;
        }
        Node* node = malloc(sizeof(node));
        if(node == NULL) {
            free(node);
            return EXECUTE_MEMORY_ERROR;
        }

        node->tableData = key_value_to_insert;
        node->next = head;
        bucket->head = node;
        bucket->numOfElements++;
    }
    

    return EXECUTE_SUCCESS;
}

ExecuteResult executeGet(Statement* statement, Bucket* table, int client) {
    char* key = ((char *) &(statement->data));

    int hashIndex = hash(key);

    Bucket bucket = table[hashIndex];
    Node *head = bucket.head;

    for(Node* n = head; n != NULL; n = n->next) {
        if(strcmp(key, n->tableData.key) == 0) {
            if(checkIfExpired(n->tableData.expiration)) {
                sendDataToClient(client, "Record not found");
                executeDelete(statement, table);
                break;
            }
            sendDataToClient(client, n->tableData.value);
            return EXECUTE_SUCCESS;
        }
    }
    
    sendDataToClient(client, "Record not found");
    return EXECUTE_SUCCESS;
}

ExecuteResult executeDelete(Statement* statement, Bucket* table) {
    char* key = ((char *) &(statement->data));

    int hashIndex = hash(key);
    Bucket *bucket = &table[hashIndex];

    Node* tableElement = bucket->head;

    if(tableElement->tableData.key == key) {
        bucket->head = tableElement->next;
        bucket->numOfElements--;
        free(tableElement);
    }
    
    for(Node* n = tableElement->next; n != NULL; n = n->next, tableElement = tableElement->next) {
        if(strcmp(key, n->tableData.key) == 0) {
            tableElement->next = n->next;
            bucket->numOfElements--;
            free(n);
            return EXECUTE_SUCCESS;
        }
    }

    return EXECUTE_SUCCESS;
}

void sendDataToClient(int client, const char* data) {
    send(client, data, strlen(data), 0);
    return;
}

bool checkIfExpired(struct tm date) {
    time_t t = time(NULL);
    struct tm now = *localtime(&t);

    if(now.tm_mday > date.tm_mday) {
        return true;
    }

    if(now.tm_hour > date.tm_hour) {
        return true;
    }

    if(now.tm_min > date.tm_min) {
        return true;
    }

    if(now.tm_sec >= date.tm_sec) {
        return true;
    }

    return false;
}