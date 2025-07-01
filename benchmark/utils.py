import random, string, csv, sys

def generate_kv_pairs(n=100000):
    return [
        (f"".join(random.choices(string.ascii_letters, k=10)), ''.join(random.choices(string.ascii_letters, k=20))) for i in range(n)
    ]

def generate_csv(n):
    with open('data.csv', 'w', newline='') as csvfile:
        writer = csv.writer(csvfile)
        data = generate_kv_pairs(n)
        writer.writerows(data)

def main():
    num = int(sys.argv[1]) if len(sys.argv) > 1 else 1000
    generate_csv(num)

if __name__ == "__main__":
    main()
    