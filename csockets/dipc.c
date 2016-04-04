/*
    C socket server example, handles multiple clients using threads
    Borrowed from:
    http://www.binarytides.com/server-client-example-c-sockets-linux/
*/
 
#include <stdio.h>
#include <string.h>    //strlen
#include <stdlib.h>    //strlen
#include <sys/socket.h>
#include <arpa/inet.h> //inet_addr
#include <unistd.h>    //write
#include <pthread.h> //for threading , link with lpthread
 
// Fucntion prototypes
void* connection_handler( void* );
int message_handler( char*, int );
char* remove_newline( char* );
int read_handler( char*, int );
int write_handler( char*, char* );
int isInt( char* );

// Parameters passed in
int NUM_MAILBOX = 0;
int MAILBOX_SIZE = 0;
int PORT = 8888;
int PACKET_SIZE = 0;
char** MAILBOX;


int main( int argc, char** argv )
{
    // check for usage
    if( argc != 5 )
    {
        printf( "Usage: dipc <num mailboxes> <mailbox sixe KB> <port num> <size of packets>\n");
        return -1;
    }
    
    // assign the parameter values
    NUM_MAILBOX = atoi( argv[1] );
    MAILBOX_SIZE = atoi( argv[2] );
    PORT = atoi( argv[3] );
    PACKET_SIZE = atoi( argv[4] );

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
        if( ( MAILBOX[i] = malloc( MAILBOX_SIZE * sizeof( char* ) ) ) == NULL )
        {
            printf( "Error allocating mailboxes!\n" );
            return -1;
        }
    }

    int socket_desc, client_sock, c;
    int* new_sock;
    struct sockaddr_in server, client;
     
    //Create socket
    socket_desc = socket( AF_INET, SOCK_STREAM, 0 );
    if ( socket_desc == -1 )
    {
        printf( "Could not create socket" );
    }
    puts( "Socket created" );
     
    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( 8888 );
     
    //Bind
    if( bind( socket_desc, (struct sockaddr *)&server , sizeof( server ) ) < 0 )
    {
        //print the error message
        perror( "Bind failed. Error!" );
        return 1;
    }
    puts( "Bind done!" );
     
    //Listen
    listen( socket_desc , 3 );

    //Accept and incoming connection
    puts( "Waiting for incoming connections..." );
    c = sizeof( struct sockaddr_in );
    while( ( client_sock = accept( socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) )
    {
        puts( "Connection accepted!" );
         
        pthread_t sniffer_thread;
        new_sock = malloc(1);
        *new_sock = client_sock;
         
        if( pthread_create( &sniffer_thread , NULL ,  connection_handler , (void*) new_sock) < 0)
        {
            perror( "Could not create thread." );
            return -1;
        }
         
        //Now join the thread , so that we dont terminate before the thread
        //pthread_join( sniffer_thread , NULL);
        puts( "Handler assigned!" );
    }

    ///huh?
    printf( "Do I ever leave thsi loop?\n" );
     
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
    char* message; 
    char client_message[2000];
     
    //Send some messages to the client
    message = "Greetings! I am your connection handler\n";
    write( sock , message , strlen( message ) );     
    message = "Now type something and i shall repeat what you type \n";
    write( sock , message , strlen( message ) );
     
    printf( "Socket: %d\n", sock );
    int flag = 1;

    // Receive a message from client
    // This continues looping until the client shuts down
    while( ( read_size = recv( sock , client_message , 2000 , 0 ) ) > 0 )
    {
        // Deal with the message here
        //printf( "Client's message %s", client_message );
        flag = message_handler( client_message, sock );
        // Send the message back to client
        //write( sock , client_message , strlen( client_message ) );
        
        // if the user is quiting
        if( flag != 1 )
        {
            read_size = 0;
            break;
        }
    }
     
    if( read_size == 0 )
    {
        puts( "Client disconnected" );
        fflush( stdout );
    }

    else if( read_size == -1 )
    {
        perror( "Recv failed" );
    }
         
    //Free the socket pointer
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
    int box_num = -1;
    
    // Grab the first command, the mailbox number (read and write)
    // and the message (for write only)
    token = strtok( msg, delim );
    box_char = strtok( NULL, delim );   
    write_msg = strtok( NULL, "" );

    //fflush( stdout );

    // check to see if they're calling it quits
    if( strcmp( token, "q" ) == 0 )
    {
        return 0;
    }

    // if we're writing to a mailbox
    else if( strcmp( token, "w" ) == 0 )
    {
        printf( "Writing...\n"); 
        write_handler( write_msg, box_char );
    }

    // if we're reading from a mailbox
    else if( strcmp( token, "r" ) == 0 )
    {
        printf( "Reading...\n" );
        read_handler( box_char, sock );
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

    // If the number given from the user isn't even a number at all... idiot
    if( !isInt( box_char ) )
    {
        printf( "Invalid mailbox number!\n" );
        return 1;
    }

    // convert the given number to an int
    box_num = atoi( box_char );

    // check to see if it's actually a mailbox
    if( box_num < 0 || box_num > NUM_MAILBOX )
    {
        printf( "Mailbox does not exist!\n" );
        return 1;
    }

    // grab that message
    msg = MAILBOX[box_num];

    // send it to the client
    write( sock , msg , strlen( msg ) );

    // wipe that mailbox out!
    MAILBOX[box_num][0] = '\0';

    return 1;
}

int write_handler( char* msg, char* box_char )
{
    // the box number it's trying to write to
    int box_num = -1;
    
    // if the box number passed in isn't a number at all... idiots.
    if( !isInt( box_char ) )
    {
        printf( "Invalid mailbox number!\n" );
        return 1;
    }

    // convert it to an int
    box_num = atoi( box_char );

    // make sure the mailbox they're looking for is actually a mailbox
    if( box_num < 0 || box_num > NUM_MAILBOX )
    {
        printf( "Mailbox does not exist!\n" );
        return 1;
    }

    // write the message into that mailbox
    MAILBOX[box_num] = msg;

    return 1;
}

char* remove_newline( char* line )
{
    printf( "checking int\n" );
    size_t len = strlen( line );
    if( len > 0 && line[len-1] == '\n' )
    {
        line[--len] = '\0';
    }

    return line;
}

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

