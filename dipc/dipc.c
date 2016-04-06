/*
    C socket server example, handles multiple clients using threads
    Borrowed from:
    http://www.binarytides.com/server-client-example-c-sockets-linux/
*/
 
#include <stdio.h>
#include <string.h>    // strlen
#include <stdlib.h>    // strlen
#include <sys/socket.h>
#include <arpa/inet.h> // inet_addr
#include <unistd.h>    // write
#include <pthread.h> // for threading , link with lpthread
 
// Fucntion prototypes
void* connection_handler( void* );
int message_handler( char*, int );
//char* remove_newline( char* );
int read_handler( char*, int );
int write_handler( char*, char*, int );
int isInt( char* );
void initiate_shutdown();

// Parameters passed in
int NUM_MAILBOX = 0;
int MAILBOX_SIZE = 0;
int PORT = 8888;
int PACKET_SIZE = 0;

int SHUTDOWN_ME = 0;

// THE mailbox and its locks
char** MAILBOX;
pthread_mutex_t* MAILBOX_LOCK;

int main( int argc, char** argv )
{
    // check for usage
    if( argc != 5 )
    {
        printf( "Usage: dipc <num mailboxes> <mailbox size KB> <port num> <size of packets KB>\n");
        return -1;
    }
    
    // assign the parameter values
    NUM_MAILBOX = atoi( argv[1] );
    MAILBOX_SIZE = atoi( argv[2] );
    PORT = atoi( argv[3] );
    PACKET_SIZE = atoi( argv[4] );

    if( PACKET_SIZE > MAILBOX_SIZE )
    {
        printf( "Error: Packetsize too large for mailbox.\n" );
        return -1;
    }

    // allocate the mailboxes
    ///figure out deallocation later
    if( ( MAILBOX = malloc( NUM_MAILBOX * sizeof( char* ) ) ) == NULL )
    {
        printf( "Error allocating mailboxes!\n" );
        return -1;
    }
    int i;
    for( i = 0; i < NUM_MAILBOX; i++ )
    {
        // 1 byte in a char, 1024 bytes in a KB
        if( ( MAILBOX[i] = malloc( MAILBOX_SIZE * sizeof( char* ) * 1024 ) ) == NULL )
        {
            printf( "Error allocating mailboxes!\n" );
            return -1;
        }
    }

    // allocate the locks for the mailboxes
    if( ( MAILBOX_LOCK = malloc( NUM_MAILBOX * sizeof( pthread_mutex_t ) ) ) == NULL )
    {
        printf( "Error allocating locks for the mailboxes!\n" );
        return -1;
    }

    // initialize the locks
    for( i = 0; i < NUM_MAILBOX; i++ )
    {
        pthread_mutex_init( &MAILBOX_LOCK[i], NULL );
    }

    int socket_desc, client_sock, c;
    int* new_sock;
    struct sockaddr_in server, client;
     
    // Create socket
    socket_desc = socket( AF_INET, SOCK_STREAM, 0 );
    if ( socket_desc == -1 )
    {
        printf( "Could not create socket" );
    }
    
    // Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( PORT );
     
    // Bind
    if( bind( socket_desc, (struct sockaddr *)&server , sizeof( server ) ) < 0 )
    {
        // print the error message
        perror( "Bind failed. Error!" );
        return -1;
    }

    daemon( 1, 1 );

    // Listen
    listen( socket_desc , 3 );

    // Accept and incoming connection
    c = sizeof( struct sockaddr_in );
    while( ( client_sock = accept( socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) )
    {    
        pthread_t sniffer_thread;
        new_sock = malloc(1);
        *new_sock = client_sock;
         
        if( pthread_create( &sniffer_thread , NULL ,  connection_handler , (void*) new_sock) < 0)
        {
            perror( "Could not create thread." );
            return -1;
        }
    }
     
    if( client_sock < 0 )
    {
        perror( "Accept failed." );
        return -1;
    }
     
    return 0;
}

/*
 * This will handle connection for each client
 * */
void* connection_handler( void* socket_desc )
{
    //Get the socket descriptor
    int sock = *(int*)socket_desc;
    int read_size;
    ///char* message; 
    char client_message[2000];

    int flag = 1;

    // Receive a message from client
    // This continues looping until the client shuts down
    while( ( read_size = recv( sock , client_message , 2000 , 0 ) ) > 0 )
    {
        if( SHUTDOWN_ME == 1 )
        {
            free( socket_desc );
            return 0;
        }


        // if the user sent a message that's too large
        if( strlen( client_message ) > PACKET_SIZE * 1024 )
        {
            char* pack_size_error = "Packet size too large. Unable to write.\n";
            write( sock, pack_size_error, strlen( pack_size_error ) );           
        }
        else
        {
            // Deal with the message here
            flag = message_handler( client_message, sock );
        }

        // if the user is quiting
        if( flag == 0 )
        {
            read_size = 0;
            break;
        }
        

        // if we're shutting down the socket
        if( flag == -1 )
        {
            SHUTDOWN_ME = 1;
            initiate_shutdown( sock );
        }
    }
     
    // client disconnected
    if( read_size == 0 )
    {
        fflush( stdout );
    }

    else if( read_size == -1 )
    {
        perror( "Recv failed" );
    }
         
    // Free the socket pointer
    free( socket_desc );
     
    return 0;
}

int message_handler( char* msg, int sock )
{ 
    // parse the message
    const char delim[4] = " \t\n";
    char* token = NULL;
    char* box_char;
    char* write_msg;    
    
    // Grab the first command, the mailbox number (read and write)
    // and the message (for write only)
    token = strtok( msg, delim );
    box_char = strtok( NULL, delim );   
    write_msg = strtok( NULL, "" );

    // check to see if they're calling it quits
    if( strcmp( token, "q" ) == 0 )
    {
        return 0;
    }

    // if we're writing to a mailbox
    else if( strcmp( token, "w" ) == 0 )
    { 
        write_handler( write_msg, box_char, sock );
    }

    // if we're reading from a mailbox
    else if( strcmp( token, "r" ) == 0 )
    {
        ///printf( "Reading...\n" );
        read_handler( box_char, sock );
    }

    // if we're shutting down the server
    else if( strcmp( token, "RM" ) == 0 )
    {
        return -1;
    }

    // ignore all other bullshit
    else
    {
        return 1;
    }
 
    return 1;
}

int read_handler( char* box_char, int sock )
{
    // the mailbox number to read from
    int box_num = -1;

    // holds the message in the box to be printed to the client
    char* msg;

    // error statement if the user provides an invalid number
    char* num_error = "Invalid mailbox number.\n";

    // If the number given from the user isn't even a number at all... idiot
    if( !isInt( box_char ) )
    {
        write( sock, num_error, strlen( num_error ) );
        return 1;
    }

    // convert the given number to an int
    box_num = atoi( box_char );
    
    // convert for csc friendly array accessing :)
    box_num = box_num - 1;

    // check to see if it's actually a mailbox
    if( box_num < 0 || box_num >= NUM_MAILBOX )
    {
        write( sock, num_error, strlen( num_error ) );
        return 1;
    }

    // lock the mutex lock to THAT mailbox
    pthread_mutex_lock( &MAILBOX_LOCK[box_num] );
    
    // grab that message
    msg = MAILBOX[box_num];

    // send it to the client
    write( sock , msg , strlen( msg ) );

    // wipe that mailbox out!
    MAILBOX[box_num][0] = '\0';

    // now that we're done here, we can unlock this mailbox
    pthread_mutex_unlock( &MAILBOX_LOCK[box_num] );

    return 1;
}

int write_handler( char* msg, char* box_char, int sock )
{
    // the user didn't provide a message. Return unless you want a seggy fault
    if( msg == NULL )
    {
        return 1;
    }

    // the box number it's trying to write to
    int box_num = -1;
 
    // error statement for when the user provides an invalid mailbox number   
    char* num_error = "Invalid mailbox number.\n";

    // if the box number passed in isn't a number at all... idiots.
    if( !isInt( box_char ) )
    {
        write( sock, num_error, strlen( num_error ) );
        return 1;
    }

    // convert it to an int
    box_num = atoi( box_char );

    // convert for csc friendly array accessing :)
    box_num = box_num - 1;

    // make sure the mailbox they're looking for is actually a mailbox
    if( box_num < 0 || box_num >= NUM_MAILBOX )
    {
        write( sock, num_error, strlen( num_error ) );       
        return 1;
    }

    // lock this mailbox with its pthread mutex
    pthread_mutex_lock( &MAILBOX_LOCK[box_num] );

    // write the message into that mailbox
    MAILBOX[box_num] = msg;

    // now that we're done, we can unlock it
    pthread_mutex_unlock( &MAILBOX_LOCK[box_num] );

    return 1;
}

void initiate_shutdown( int sock_desc )
{
    int i;
    
    // close the socket   
    close( sock_desc );
    
    // destroy the threads
    for( i = 0; i < NUM_MAILBOX; i++ )
    {
        pthread_mutex_destroy( &MAILBOX_LOCK[i] );
    }

    // deallocate the mailboxes 
    free( MAILBOX );

    exit(0);

    return;
}
/*
// Is given a char string, checks to see if there's a newline character
// at the end. If so, removes it then returns it.
char* remove_newline( char* line )
{
    size_t len = strlen( line );
    if( len > 0 && line[len-1] == '\n' )
    {
        line[--len] = '\0';
    }

    return line;
}
*/
// Checks to see if a character is an int by comparing the ascii values
// is between 48 (0) and 57 (9). Also checks for negative number
// Param: char* num - character string to be check if an integer
// Return: -1 - not an int, 1 - it's an int
int isInt( char* num )
{
    int i = 0;

    // if it's a negative number, ignore the - sign
    if( num[0] == 45 )
    {
        i = 1;
    }

    // go through each decimal in the number passed in
    while( num[i] )
    {
        // if it's in the boundaries of an int
        if( num[i] < 48 || num[i] > 57 )
        {
            return -1;
        }

        i++;
    }

    // else, it's an int
    return 1;
}


