// YOUR NAME HERE
// HTTP Client - Computer Networks


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


    /* Setup socket. */
    int sock = common_createSocket(argv[1], argv[2]);
    FILE *stream = common_getStream(sock);

    /* TODO: Write HTTP request to "stream" */


    while(1) // while there is data to read
    {
	char *string = NULL;
	int len = common_getLine(&string, stream);

	// TODO: Process the line that we have read.


	// TODO: When we reach the end of stream, fclose(stream), print
	// message that the connection was closed to console, and
	// exit(EXIT_SUCCESS).


	free(string);

    } // end infinite loop

}
