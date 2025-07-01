import socket, time
import csv

HOST = "localhost"
PORT = 8080

with open('data.csv', 'r') as csvfile:
    reader = csv.reader(csvfile)
    data = [(row[0], row[1]) for row in reader]

count = len(data)

client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
client.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
client.connect((HOST, PORT))

def run_benchmark(command):
    start = time.time()

    for key, value in data:
        if command == "set":
            client.sendall(f"set {key} {value}\n".encode())
            client.recv(4096)
        elif command == "get":
            client.sendall(f"get {key}\n".encode())
            client.recv(4096)

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

client.close()