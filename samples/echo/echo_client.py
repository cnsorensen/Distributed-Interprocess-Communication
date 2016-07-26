# echo_client.py
# Borrowed from:
#   http://www.bogotobogo.com/python/python_network_programming_server_client.php

import socket

host = socket.gethostname()

# Same port as server
port = 12345

s = socket.socket( socket.AF_INET, socket.SOCK_STREAM )
s.connect( ( host, port ) )
s.sendall( b'Hello, world' )
data = s.recv( 1024 )
s.close()
print( 'Received', repr( data ) )
