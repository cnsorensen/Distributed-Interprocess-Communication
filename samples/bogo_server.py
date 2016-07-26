# server.py
# Borrowed from:
# http://www.bogotobogo.com/python/python_network_programming_server_client.php

import socket
import time

# create a socket object
serversocket = socket.socket( socket.AF_INET, socket.SOCK_STREAM )

# get local machine name
host = socket.gethostname()

# bind to the port
port = 50007
serversocket.bind( ( host, port ) )

# queue up to 5 requests
serversocket.listen( 5 )

while True:
    
    # establish a connect
    clientsocket, addr = serversocket.accept()

    print( "Got a connection for %s" % str( addr ) )
    
    currentTime = time.ctime( time.time() ) + "\r\n"
    clientsocket.send( currentTime.encode( 'ascii' ) )
    clientsocket.close()

