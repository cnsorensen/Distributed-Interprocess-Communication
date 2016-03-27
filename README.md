# dipc

DIPC - Distributed Interprocess Communication tool: 

The third programming assignment is to write a distributed interprocess communication tool.  This can be thought of as a middleware shared memory system.   The DIPC tool will provide shared memory for storage and interprocess communication through a socket interface.    The shell will start a server process which will setup the shared memory, the required semaphores to control access to the shared memory and create the sockets.   Since multiple processes will have access to the shared memory, the standard shared resource problems will arise.   You must resolve the multiple access (or starvation and deadlock) issues that can occur.    This program will allow a group of process to communicate with each other; each one connected as a client to a central server.   Thus, the clients only need simple client socket code.   A simple chat program would use this. 

Use of the system will be:

    The user will start the server on the command line specifying number of mailboxes, size of a mail box, the communication port and the size of a communication packet.
    A client program will connect to the server and get handed off to a thread to manage the communication.
    The communication thread will access the shared memory in a safe manner.  It will move data back and forth between the shared memory and the client process (via a socket). 
    When finished, the user will issue a server shutdown.

The communication thread will be both reading and writing to the shared memory.   Since there can be multiple threads running, there can be multiple readers and writers.  You will allow more than one reader but only one writer in a block.   You should use pthreads for the thread library.  You may use the shared memory aspect of pthreads or you can use the System V IPC Shared Memory - your design decision. 

Required elements:

    DIPC initialization
        Usage:  dipc <number of mailboxes>  <size of mailbox in kbytes>  <port>  <size of packet in kbytes>
        Starts the dipc server
        Create k blocks
            k = number of mailboxes requested
            block size = size of the mailbox
        Create the socket server which can hand
        Returns (exits) with error message if server init fails, otherwise runs in the background like a daemon.
    DIPC removal
        Usage:  dipcrm <port>
        Clean up semaphores and shared memory if required.
        Will kill server process (stop the socket accept loop).
    DIPC server
        Will listen for connections.
        Will accept connections and hand them off to a thread.
        Will exit when it gets the exit command from dipcdel.
        Will establish the shared memory component.
    DIPC Communication thread
        Interact with client over socket setup by server.
        Interact with shared memory and solving the critical section problem (no starvation, no deadlocks). 
    DIPC datagram
        First line will have a "r" for read, "w" for write, or "q" for quit and the mailbox number.
        Note that "q" will kill the connection to the specific client, not kill the server.
        Following the first line is data (void for "q").  

