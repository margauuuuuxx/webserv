import requests, threading

def worker():
    for _ in range(1000):
        try:
            r = requests.get("http://127.0.0.1:8080/cat.jpg")
            print(r.status_code)
        except Exception as e:
            print("error:", e)

threads = []
for _ in range(50):  # 50 threads
    t = threading.Thread(target=worker)
    t.start()
    threads.append(t)

for t in threads:
    t.join()
