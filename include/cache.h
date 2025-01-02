#ifndef CACHE_H_
#define CACHE_H_

#define BUFFER_SIZE 50

typedef enum {
    PREPARE_SUCCESS, //keyword for representing a succesfull query statement
    PREPARE_UNRECOGNIZED_STATEMENT //keyword for representing a malformed or wrong query
} StatementResult;

typedef enum {
    CACHE_SET, //keyword for setting a key-value pair
    CACHE_DELETE //keyword for deleting a key-value pair
} CacheAction;

typedef struct {
    CacheAction action;
} Statement;

void *handleRequest(void *arg);

StatementResult prepareStatement(char* buffer, Statement* action);

void executeAction(CacheAction* action);

#endif  