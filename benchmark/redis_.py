import time
from utils import generate_kv_pairs

import redis

data = generate_kv_pairs(400000)

HOST = "localhost"
PORT = 8080

client = redis.Redis(host=HOST, port=6379, db=0)

set_start = time.time()
i = 1
for key, value in data:
    res = client.set(key, value)

set_end = time.time()
set_duration = set_end - set_start

get_start = time.time()
for key, _ in data:
    response = client.get(key)
    print(response)
get_end = time.time()
get_duration = get_end - get_start

print(f"Set {len(data)} key-value pairs in {set_duration:.2f} seconds")
print(f"Get {len(data)} key-value pairs in {get_duration:.2f} seconds")
