import socket, time
from utils import generate_kv_pairs

data = generate_kv_pairs(400000)

HOST = "localhost"
PORT = 8080

client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
client.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
client.connect((HOST, PORT))

set_start = time.time()
for key, value in data:
    client.sendall(f"set {key} {value}\n".encode())
    client.recv(4096)

set_end = time.time()
set_duration = set_end - set_start


get_start = time.time()
i = 1
for key, _ in data:
    client.sendall(f"get {key}\n".encode())
    print(i, client.recv(4096))
    i += 1

get_end = time.time()
get_duration = get_end - get_start

print(f"Set {len(data)} key-value pairs in {set_duration:.2f} seconds")
print(f"Get {len(data)} key-value pairs in {get_duration:.2f} seconds")

client.close()