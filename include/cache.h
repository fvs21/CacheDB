#ifndef CACHE_H_
#define CACHE_H_

#include <time.h>
#include <stdbool.h>
#include <pthread.h>

#define BUFFER_SIZE 600

#define KEY_SIZE 50 //bytes
#define VALUE_SIZE 500

#define BUCKETS 1024

#define CACHE_CAPACITY 55000000 //0.5 MB

typedef struct {
    char key[KEY_SIZE];
    char value[VALUE_SIZE];
} TableData;

typedef enum {
    PREPARE_SUCCESS, //keyword for representing a succesfull query statement
    PREPARE_UNRECOGNIZED_STATEMENT, //keyword for representing an unrecognized command
    PREPARE_SYNTAX_ERROR //keyword for representing a syntax error
} StatementResult;

typedef enum {
    EXECUTE_SUCCESS, //keyword for a succesfull insertion or deletion
    EXECUTE_CACHE_FULL, //keyword for full cache table
    EXECUTE_MEMORY_ERROR //keyword for a memory error
} ExecuteResult;

typedef enum {
    CACHE_SET, //keyword for setting a key-value pair
    CACHE_DELETE, //keyword for deleting a key-value pair
    CACHE_GET, //key word for retrieving a value
    CACHE_EVICT //keyword for evicting the LRU item
} CacheAction;

typedef struct {
    CacheAction action;
    void* data;
} Statement;

typedef struct DLLNode {
    struct DLLNode* next;
    struct DLLNode* prev;
    struct BucketNode* bucketNode;
} DLLNode;

typedef struct BucketNode {
    TableData tableData;
    DLLNode* lruNode;
    struct BucketNode* next;
} BucketNode;

typedef struct Bucket {
    BucketNode* head;
} Bucket;


typedef struct {
    Bucket table[BUCKETS];
    DLLNode* head;
    DLLNode* tail;
    size_t current_memory;
} Cache;

Cache* cache;
extern pthread_mutex_t mutex;

void *handleRequest(void *arg);

StatementResult prepareStatement(char* buffer, Statement* statement);

void initializeCache();
ExecuteResult executeSet(Statement* statement);
ExecuteResult executeDelete(Statement* statement);
ExecuteResult executeGet(Statement* statement, int client);
ExecuteResult executeEvict();
void sendDataToClient(int client, const char* data);
unsigned int hash(const char* key);
bool deleteItem(char* key);

DLLNode* createDLLNode(BucketNode* node);
void evictLRU();
void moveToHead(DLLNode* node);
void deleteFromLRUList(DLLNode* node);

#endif  