import csv
import time
import psycopg2

with open('data.csv', 'r') as csvfile:
    reader = csv.reader(csvfile)
    data = [(row[0], row[1]) for row in reader]

count = len(data)

conn = psycopg2.connect("dbname=benchmark user=tester password=testeruser")
c = conn.cursor()

c.execute("CREATE TABLE IF NOT EXISTS cache (key TEXT PRIMARY KEY, value TEXT)")
conn.commit()

def run_benchmark(command):
    start = time.time()

    if command == "set":
        for key, value in data:
            c.execute("INSERT INTO cache (key, value) VALUES (%s, %s) ON CONFLICT (key) DO UPDATE SET value = EXCLUDED.value", (key, value))
    elif command == "get":
        for key, _ in data:
            c.execute("SELECT value FROM cache WHERE key = %s", (key,))
            c.fetchone()

    conn.commit()

    end = time.time()
    duration = end - start
    latency = (duration / count) * 1e6
    throughput = count / duration

    print(f"{command.capitalize()} {count} key-value pairs in {duration:.5f} seconds")
    print(f"Time: {duration:.5f} seconds")
    print(f"Latency: {latency:.2f} microseconds")
    print(f"Throughput: {throughput:.2f} operations/second")

run_benchmark("set")
run_benchmark("get")

c.execute("DELETE FROM cache")
conn.commit()