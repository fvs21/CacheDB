# CacheDB

Simple <a src="https://memcached.org/">Memcached</a> clone in C

## Description
<a src="https://memcached.org/">Memcached</a> is an open source distributed memory caching system, mostly used in 
web applications to speed up queries, and avoid unnecesary calls to a persistent database. 

The idea is to cache data that is known to be accessed frequenly, storing a key value pair in a hash table data structure.

The server executable, opens a socket connection to receive data from the client.
Then, it parses the command and executes the corresponding action.

The cache supports 3 commands: set, get, delete

## Usage
### For the server

```
cd src
make
./cache

```

### For the client
```
cd src
make
./client

```

## Commands for client: 

### Setting a value
```
cache > set "key" "value"
```

### Getting a value
```
cache > get "key"
```

### Deletting a value
```
cache > delete "key`
```
