import random, string

def generate_kv_pairs(n=100000):
    return [
        (f"key_{i}", ''.join(random.choices(string.ascii_letters, k=20))) for i in range(n)
    ]