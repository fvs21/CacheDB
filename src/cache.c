#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include "cache.h"
#include <time.h>

static int total_commands = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void initializeCache() {
    cache = malloc(sizeof(Cache));

    if(cache == NULL) {
        perror("Failed to initialize cache");
        exit(EXIT_FAILURE);
    }

    DLLNode* head = malloc(sizeof(DLLNode));
    DLLNode* tail = malloc(sizeof(DLLNode));
    if(head == NULL || tail == NULL) {
        perror("Failed to initialize cache");
        exit(EXIT_FAILURE);
    }
    head->next = tail;
    tail->prev = head;
    cache->head = head;
    cache->tail = tail;
    cache->current_memory = 0;

    for(int i = 0; i<BUCKETS; i++) {
        Bucket bucket = {NULL};
        cache->table[i] = bucket;
    }
}

void *handleRequest(void *arg) {
    int client = *((int *) arg);
    char buffer[BUFFER_SIZE];

    while(1) {
        ssize_t bytesRecieved = recv(
            client, //receive this client request data
            &buffer, //buffer to store incoming data
            BUFFER_SIZE, 
            0
        );

        if(bytesRecieved <= 0) {
            continue;
        }

        Statement statement;
            
        StatementResult statementResult = prepareStatement(buffer, &statement);

        switch(statementResult) {
            case(PREPARE_SUCCESS):
                break;
            case(PREPARE_SYNTAX_ERROR):
                sendDataToClient(client, "Syntax error.\n");
                continue;
            case(PREPARE_UNRECOGNIZED_STATEMENT):
                sendDataToClient(client, "Unrecognized keyword.\n");
                continue;
        }

        ExecuteResult result;

        pthread_mutex_lock(&mutex); //lock the cache
        switch(statement.action) {
            case(CACHE_SET):
                result = executeSet(&statement);
                break;
            case(CACHE_DELETE): 
                result = executeDelete(&statement);
                break;
            case(CACHE_GET):
                result = executeGet(&statement, client);
                break;
            case(CACHE_EVICT):
                result = executeEvict();
                break;
        }
        pthread_mutex_unlock(&mutex); //unlock the cache

        switch(result) {
            case(EXECUTE_SUCCESS):
                if(statement.action == CACHE_GET) {
                    break;
                }
                sendDataToClient(client, "True\n");
                break;  
            case(EXECUTE_CACHE_FULL):
                sendDataToClient(client, "Cache full");
                break;
            case(EXECUTE_MEMORY_ERROR):
                sendDataToClient(client, "Memory error");
                break;
        }

        total_commands++;
        printf("Processed %d commands\n", total_commands);

        bzero(buffer, BUFFER_SIZE);
    }
    free(arg);
    close(client);
    pthread_detach(pthread_self());
    return NULL;
}

StatementResult prepareStatement(char* buffer, Statement* statement) {
    if(strncmp(buffer, "set", 3) == 0) {
        statement->action = CACHE_SET; //set action to set

        TableData *key_value_to_insert = (TableData *) &(statement->data); //cast statement data to TableData

        int args_assigned = sscanf(
            buffer,
            "set %50s %500c", //set "key" "value"
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
            "delete %50s",
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
            "get %250s",
            key
        );

        if(args_assigned < 1) {
            return PREPARE_SYNTAX_ERROR;
        }
        return PREPARE_SUCCESS;
    } else if(strncmp(buffer, "evict", 5) == 0) {
        statement->action = CACHE_EVICT;

        return PREPARE_SUCCESS;
    }
    return PREPARE_UNRECOGNIZED_STATEMENT;
}

unsigned int hash(const char* key) {
    unsigned long hash = 5381;
    int c;

    while((c = *key++)) {
        hash = ((hash << 5) + hash) + c;
    }

    return hash % BUCKETS;
}

ExecuteResult executeSet(Statement* statement) {
    TableData key_value_to_insert = *((TableData *) &(statement->data));
    int hash_index = hash(key_value_to_insert.key);

    size_t data_size = sizeof(key_value_to_insert); //TODO

    Bucket* bucket = &cache->table[hash_index];
    BucketNode* head = bucket->head;

    if(head == NULL) {
        while (cache->current_memory + data_size > CACHE_CAPACITY) {
            evictLRU();
        }

        BucketNode* node = malloc(sizeof(BucketNode));
        if(node == NULL) {
            free(node);
            return EXECUTE_MEMORY_ERROR;
        }
        memcpy(&(node->tableData.key), &(key_value_to_insert.key), sizeof(key_value_to_insert.key));
        memcpy(&(node->tableData.value), &(key_value_to_insert.value), sizeof(key_value_to_insert.value));
        node->next = NULL;
        bucket->head = node; //wow

        DLLNode* dllnode = createDLLNode(node);
        node->lruNode = dllnode;

    } else {
        for(BucketNode* n = bucket->head; n != NULL; n = n->next) {
            if(strcmp(n->tableData.key, key_value_to_insert.key) == 0) {
                //if key already exists, update value
                memcpy(&(n->tableData.value), &(key_value_to_insert.value), sizeof(key_value_to_insert.value));
                moveToHead(n->lruNode);
                return EXECUTE_SUCCESS;
            }
        }
        while (cache->current_memory + data_size > CACHE_CAPACITY) {
            evictLRU();
        }
        BucketNode* node = malloc(sizeof(BucketNode));
        if(node == NULL) {
            free(node);
            return EXECUTE_MEMORY_ERROR;
        }

        node->tableData = key_value_to_insert;
        node->next = bucket->head;
        bucket->head = node;

        DLLNode* dllnode = createDLLNode(node);
        node->lruNode = dllnode;
    }

    cache->current_memory += data_size;

    return EXECUTE_SUCCESS;
}

ExecuteResult executeGet(Statement* statement, int client) {
    char* key = ((char *) &(statement->data));

    int hash_index = hash(key);

    Bucket* bucket = &cache->table[hash_index]; 

    BucketNode *head = bucket->head;

    for(BucketNode* n = head; n != NULL; n = n->next) {
        if(strcmp(key, n->tableData.key) == 0) {
            moveToHead(n->lruNode);
            sendDataToClient(client, n->tableData.value);
            return EXECUTE_SUCCESS;
        }
    }
    
    sendDataToClient(client, "Record not found");
    return EXECUTE_SUCCESS;
}

ExecuteResult executeDelete(Statement* statement) {
    char* key = ((char *) &(statement->data));

    bool res = deleteItem(key);

    if(res) {
        return EXECUTE_SUCCESS;
    }

    return EXECUTE_MEMORY_ERROR;
}

ExecuteResult executeEvict() {
    evictLRU();
    return EXECUTE_SUCCESS;
}

bool deleteItem(char* key) {
    int hash_index = hash(key);
    Bucket *bucket = &cache->table[hash_index];
    BucketNode* tableElement = bucket->head;

    if(tableElement == NULL) {
        return true;
    }

    size_t data_size;

    if(strcmp(tableElement->tableData.key, key) == 0) {
        data_size = sizeof(tableElement->tableData);
        bucket->head = tableElement->next;
        deleteFromLRUList(tableElement->lruNode);
        free(tableElement);
    } else {
        for(BucketNode* n = tableElement->next; n != NULL; n = n->next, tableElement = tableElement->next) {
            if(strcmp(key, n->tableData.key) == 0) {
                data_size = sizeof(n->tableData);
                tableElement->next = n->next;
                deleteFromLRUList(n->lruNode);
                free(n);
                break;
            }
     }
    }
    
    cache->current_memory -= data_size;

    return true;
}

void sendDataToClient(int client, const char* data) {
    send(client, data, strlen(data), 0);
    return;
}

DLLNode* createDLLNode(BucketNode* node) {
    DLLNode* dllNode = malloc(sizeof(DLLNode));

    if(dllNode == NULL) {
        perror("Error creating node");
        exit(EXIT_FAILURE);
    }

    dllNode->next = NULL;
    dllNode->prev = NULL;
    dllNode->bucketNode = node;

    moveToHead(dllNode);

    return dllNode;
}

void moveToHead(DLLNode* node) {
    if(node->prev != NULL && node->next != NULL) { // check if node is recently created
        DLLNode* prev = node->prev;
        DLLNode* next = node->next;
        prev->next = next;
        next->prev = prev;
    }

    DLLNode* mru = cache->head->next;

    cache->head->next = node;
    node->prev = cache->head;
    node->next = mru;
    mru->prev = node;
}

void evictLRU() {
    DLLNode* lru = cache->tail->prev;
    printf("Evicted\n");
    
    if (lru == cache->head) {
        return;
    }

    deleteItem(lru->bucketNode->tableData.key);
}

void deleteFromLRUList(DLLNode* node) {
    DLLNode* prev = node->prev;
    DLLNode* next = node->next;

    prev->next = next;
    next->prev = prev;
    free(node);
}
