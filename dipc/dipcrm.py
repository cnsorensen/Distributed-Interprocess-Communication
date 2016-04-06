# dipcrm.py
#
# Shuts down the surver.
# Params: hostname - where the server is
#         port - the port of the server
#

import sys, socket, select
 
def dipcrm():
    if( len( sys.argv ) < 3 ):
        print 'Usage : python dipcrm.py hostname port'
        sys.exit()

    host = sys.argv[1]
    port = int( sys.argv[2] )
     
    s = socket.socket( socket.AF_INET, socket.SOCK_STREAM )
    s.settimeout( 2 )
     
    # connect to remote host
    try :
        s.connect( ( host, port ) )
    except :
        print 'Unable to connect'
        sys.exit()
     
    # Send message to server to shutdown
    msg = "AVSLUTA"
    s.send( msg )

if __name__ == "__main__":

    sys.exit( dipcrm() )


