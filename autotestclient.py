# chat_client.py
#
# This is a personal debugging extension of a client code.
# It recieves the address and port number to connect to the server
# It also recieves the number of mailboxes that the server had
# And it recieves the name of the user
# It randomizes which mailbox to go to
# And it randomizes its read and writes
# It writes a message with its name to a mailbox
# It auto does this until the user force stops the program
#

import random, sys, socket, select, time
 
def chat_client():
    if(len(sys.argv) < 5) :
        print 'Usage : python chat_client.py hostname port num_mailboxes name'
        sys.exit()

    host = sys.argv[1]
    port = int(sys.argv[2])
    n = int(sys.argv[3])     
    name = sys.argv[4]

    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.settimeout(2)
     
    # connect to remote host
    try :
        s.connect((host, port))
    except :
        print 'Unable to connect'
        sys.exit()
     
    print 'Connected to remote host. Hit enter to start automation'
    sys.stdout.write('[' + name + '] '); sys.stdout.flush()
     
    while 1:
        socket_list = [sys.stdin, s]
         
        # Get the list sockets which are readable
        read_sockets, write_sockets, error_sockets = select.select(socket_list , [], [])
         
        for sock in read_sockets:            
            if sock == s:
                # incoming message from remote server, s
                data = sock.recv(4096)
                if not data :
                    print '\nDisconnected from chat server'
                    sys.exit()
                else :
                    #print data
                    sys.stdout.write(data)
                    sys.stdout.write('[' + name + '] '); sys.stdout.flush()     
            
            else :
                # user entered a message
                #msg = sys.stdin.readline()
                #msg.rstrip('\n');
                # send the message to the server
                msg = generate_message(n, name)
                time.sleep(1);
                s.send(msg)
                sys.stdout.write('[' + name + '] '); sys.stdout.flush() 

def generate_message(n, name):

    num = random.randint(1, n)
    rw_n = random.randint(1, 2)

    if rw_n == 1:
        msg = "r " + str(num)
        print "Reading from mailbox " + str(num)
    elif rw_n == 2:
        msg = "w " + str(num) + "message from " + name + " " + str(num) + " \n"
        print "Writing to mailbox " + str(num)

    return msg;

if __name__ == "__main__":

    sys.exit(chat_client())


