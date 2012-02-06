// YOUR NAME HERE
// Computer Networks

#include "common.h"

/* Number of incoming connections to queue up while we are answering other
 * requests.  Used when we call listen(). */
#define LISTEN_QUEUE_SIZE 10


/* common_getLine() Reads a line from a stream and stores it in "string"
 * which will automatically be appropriately allocated.
 *
 * RETURNS: Number of characters in the line and stores the line in the
 * variable "string".  This may differ from strlen(string) if the string
 * contains \0.  The "string" variable should be free()'d by the caller.
 * Returns -1 when the end of the stream is reached.  Exits if some other
 * error occurs.
 *
 * You should call this function this way:
 * char *string = NULL;
 * common_getLine(&string, stream);
 *
*/
size_t common_getLine(char **string, FILE *stream)
{
    // TODO: Call the getline() function with the given string, a variable
    // for the string length to be stored, and the given stream.  Check the
    // return value for an error (see getline() documentation).  If error,
    // print message and exit.  Since it returns the same thing for an error
    // and reaching the end of the file, you will need to check if you
    // reached the end of file.  If feof(stream)!= 0, then the stream is
    // at the end of file.

	size_t len = 0; 
	size_t ret = 0;


	ret = getline(string, &len, stream); 

	if(ret == -1){
		perror("##### common_getLine");
		exit(-1);
	}

    return ret; 
}



/* Convert file descriptor into stream (i.e., FILE*).  Exit if there is a
 * failure. */
FILE* common_getStream(int sock)
{
    // TODO:
    // Call fdopen(sock, "a+b").
    // Check for errors. If one occurred, print message and exit.
    // Return the stream returned by fdopen().

	FILE* filestream = fdopen(sock, "a+b");
	if(filestream == NULL){
		perror("##### common_getStream");
		exit(-1);
	}

    return filestream;
}


/* Given a socket file descriptor for a server, this function gets a new
 * socket for a specific incoming connection.  If no incoming connections
 * are present, this function will block. */
int common_acceptConnect(int sock)
{
    // TODO:
    // Call accept(sock, NULL, NULL)
    // Check for errors. If one occurred, try again repeatedly!
    // Return socket/file descriptor that accept() created.

    return 1;
}


/* Creates a TCP socket and returns the file descriptor for it.  If
 * hostname==NULL, will create a socket listening on port. Exits if there
 * is an error.*/
int common_createSocket(char *hostname, char *port)
{
    if(hostname == NULL && port == NULL)
    {
	fprintf(stderr, "Both arguments to common_createSocket() were NULL\n");
	exit(EXIT_FAILURE);
    }

    // Request a TCP connection using the version of IP that is configured
    // on the machine.
    struct addrinfo * result;
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE|AI_ADDRCONFIG;

    // TODO: Call getaddrinfo() using hostname, port, hints and the result
    // structure.  If the hostname is NULL, getaddrinfo() returns a struct
    // appropriate for the server.  Check return value of getaddrinfo() and
    // print message and exit if an error occurs.


    // TODO: Create a socket using socket() and the "result" structure.
    // Check for an error.  If one occurs, print a message and exit.
    int sock;

    /* If we are the client, initiate the connection. */
    if(hostname != NULL)
    {

	// TODO: Call connect() with "sock" and information from the
	// "result" struct to start a TCP connection.  Check for error.  If
	// one occurs, print message and exit.
    }
    else // If we are the server
    {
	/* Prevent the server from getting stuck in the TIME_WAIT state
	 * even though the server has been killed. */
	int yes = 1;
	if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) != 0)
	{
	    perror("server: setsockopt() failed: ");
	    exit(EXIT_FAILURE);
	}

	// TODO: bind() the socket using "sock" and information in the
	// "result" structure as parameters.  Check for error.  If one
	// occurs, print message and exit.

	// TODO: Start litening for connections on the socket by using
	// listen().  Set the backlog to LISTEN_QUEUE_SIZE.  Check for
	// error.  If one occurs, print message and exit.

    }  // end server-specific stuff

    freeaddrinfo(result);
    return sock;
}
