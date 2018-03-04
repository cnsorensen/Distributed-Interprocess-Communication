# echo_server.py
# Borrowed from:
# http://www.bogotobogo.com/python/python_network_programming_server_client.php

import socket

# Symbolic name meaning all available interfaces
host = '' 

# Arvitrary non-privileged port
port = 12345

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

s.bind((host, port))

s.listen(1)

conn, addr = s.accept()
print('Connected by ', addr)

while True:
    data = conn.recv(1024)
    if not data:
        break
    conn.sendall(data)
   
conn.close()
