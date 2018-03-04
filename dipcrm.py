# dipcrm.py
#
# Shuts down the server.
# It assumes that the host is the localhost
# Params: port - the port of the server
#
# This was found as a sample client from http://www.bogotobogo.com
# It was revised and used for this project as the program to shut down the server.
#
# Usage: python dipcrm.py <port number>
#

import sys, socket, select
 
def dipcrm():
    if(len(sys.argv) < 2):
        print 'Usage : python dipcrm.py <port>'
        sys.exit()

    #host = sys.argv[1]
    # Assuming the remove is in the localhost
    host = '127.0.0.1'
    port = int(sys.argv[1])
     
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.settimeout(2)
     
    # connect to remote host
    try :
        s.connect((host, port))
    except :
        print 'Unable to connect'
        sys.exit()
     
    # Send message to server to shutdown
    msg = "RM"
    s.send(msg)

if __name__ == "__main__":

    sys.exit(dipcrm())


