import socket

s = socket.socket()
s.connect(('localhost', 8080))

print("Connected to server...")

s.send(b"CONNECT\n")

data = s.recv(1024)

if not data.startswith(b"START"):
    s.close()
    exit()

pos = 250

while True:
    s.send((str(pos) + "\n").encode())

    data = s.recv(1024)

    if data.startswith(b"WIN"):
        print("You won")
        break

    text = data.decode().strip()
    pos = int(text.split(":")[-1])
    pos -= 50
    if (pos < 0):
        pos = 0
    elif (pos > 500):
        pos = 500

s.close()
