#ifndef CACHE_H_
#define CACHE_H_

#include <time.h>
#include <stdbool.h>

#define BUFFER_SIZE 10490

#define KEY_SIZE 50 //bytes
#define VALUE_SIZE 500 //10 kb

#define BUCKET_MAX_SIZE 27500 //bytes
#define BUCKET_MAX_ELEMS 50

#define BUCKETS 1024

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
    CACHE_GET //key word for retrieving a value
} CacheAction;

typedef struct {
    CacheAction action;
    void* data;
} Statement;

typedef struct Node {
    TableData tableData;
    struct Node* next;
} Node;

typedef struct {
    Node* head;
    int numOfElements;
} Bucket;

Bucket table[BUCKETS];

void *handleRequest(void *arg);

StatementResult prepareStatement(char* buffer, Statement* statement);

struct tm calculateCacheExpirationDate(float milliseconds);

void initializeCache();
ExecuteResult executeSet(Statement* statement, Bucket* table);
ExecuteResult executeDelete(Statement* statement, Bucket* table);
ExecuteResult executeGet(Statement* statement, Bucket* table, int client);
void sendDataToClient(int client, const char* data);
unsigned int hash(const char* key);

#endif  