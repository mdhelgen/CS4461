// YOUR NAME HERE
// HTTP Server - Computer Networks


#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <string.h>

#include "common.h"




int main(int argc, char *argv[])
{
    /* TODO: Check number of arguments */

    // Create a socket to listen on:
    int sock = common_createSocket(NULL, argv[1]);

    /* Repeatedly wait for new connections. */
    while(1)
    {
	printf("\nWaiting for connection...\n");
	int newsock = common_acceptConnect(sock); // block until new connection
	FILE *stream = common_getStream(newsock);
	printf("Connection started...\n");

	// Read line from client
	char *line = NULL;
	common_getLine(&line, stream);

	// TODO: Process line from client.

	// TODO: If line begins with "GET / HTTP", answer with message
	// given as command-line parameter.  Close connection after message
	// is sent.

	// TODO: If some other message received beginning with GET, send
	// HTTP 404 response.  For example, "GET /favicon.ico HTTP/1.1".
	// Close connection after message is sent.

	// TODO: If message doesn't begin with "GET", close connection
	// without sending 404 response.

	free(line);

    }  // end: main server loop



    return 0;
}
