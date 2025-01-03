#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include "cache.h"

void initializeCache() {
    for(int i = 0; i<BUCKETS; i++) {
        Bucket bucket = {NULL, 0};
        table[i] = bucket; 
    }
}

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
                if(statement.action == CACHE_GET) {
                    break;
                }

                sendDataToClient(client, "Success\n");
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
    
    return NULL;
}

StatementResult prepareStatement(char* buffer, Statement* statement) {
    if(strncmp(buffer, "set", 3) == 0) {
        statement->action = CACHE_SET; //set action to set

        TableData *key_value_to_insert = (TableData *) &(statement->data); //cast statement data to TableData

        int args_assigned = sscanf(
            buffer,
            "set %s %s", //set "key" "value"
            (key_value_to_insert->key),
            (key_value_to_insert->value)
        );

        if(args_assigned < 2) {
            return PREPARE_SYNTAX_ERROR;
        }

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
        memcpy(&(node->tableData.key), &(key_value_to_insert.key), sizeof(key_value_to_insert.key));
        memcpy(&(node->tableData.value), &(key_value_to_insert.value), sizeof(key_value_to_insert.key));
        //*node->tableData.expiration = key_value_to_insert.expiration;
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

    if(tableElement == NULL) {
        return EXECUTE_SUCCESS;
    }

    if(strcmp(tableElement->tableData.key, key) == 0) {
        bucket->head = tableElement->next;
        bucket->numOfElements--;
        free(tableElement);
        return EXECUTE_SUCCESS;
    }
    
    for(Node* n = tableElement->next; n != NULL; n = n->next, tableElement = tableElement->next) {
        if(strcmp(key, n->tableData.key) == 0) {
            tableElement->next = n->next;
            bucket->numOfElements--;
            free(n);
            break;
        }
    }

    return EXECUTE_SUCCESS;
}

void sendDataToClient(int client, const char* data) {
    send(client, data, strlen(data), 0);
    return;
}