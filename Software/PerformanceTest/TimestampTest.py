import time

for i in range(10):
    timestamp1 = time.time_ns()
    timestamp2 = time.time_ns()
    print(f"Timestamp 1: {timestamp1}", f"Timestamp 2: {timestamp2}", f"Time difference: {timestamp2 - timestamp1}", sep="\n")
    time.sleep(1)
