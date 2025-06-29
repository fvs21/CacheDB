from utils import generate_kv_pairs
import time
import psycopg2

data = generate_kv_pairs(400000)

conn = psycopg2.connect("dbname=benchmark user=tester password=testeruser")
c = conn.cursor()

c.execute("CREATE TABLE IF NOT EXISTS cache (key TEXT PRIMARY KEY, value TEXT)")
conn.commit()

set_start = time.time()
for key, value in data:
    c.execute("INSERT INTO cache (key, value) VALUES (%s, %s) ON CONFLICT (key) DO UPDATE SET value = EXCLUDED.value", (key, value))
conn.commit()

set_end = time.time()
set_duration = set_end - set_start

get_start = time.time()
for key, _ in data:
    c.execute("SELECT value FROM cache WHERE key = %s", (key,))
    result = c.fetchone()
    if result:
        print(f"Key: {key}, Value: {result[0]}")
get_end = time.time()
get_duration = get_end - get_start

print(f"Set {len(data)} key-value pairs in {set_duration:.2f} seconds")
print(f"Get {len(data)} key-value pairs in {get_duration:.2f} seconds")

c.execute("DELETE FROM cache")
conn.commit()