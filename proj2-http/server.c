// Matt Helgen
// HTTP Server - Computer Networks


#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <string.h>

#include "common.h"

#define __CHK_RET(); if(ret<0){perror("@@@@@ server encountered error writing to socket");}


int main(int argc, char *argv[])
{

	int ret;

    /* TODO: Check number of arguments */
    if (argc < 3){
    	printf("\tUsage: ./server <port> <message>\n");
	exit(EXIT_FAILURE);
    } 

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



/*   // test case for client with chunked encoding
	fprintf(stream, "HTTP/1.1 200 OK\r\n");
	fprintf(stream, "Content-Type: text/plain\r\n");
	fprintf(stream, "Transfer-Encoding: chunked\r\n");
	fprintf(stream, "\r\n");
	fprintf(stream, "25\r\n");
	fprintf(stream, "This is the data in the first chunk\r\n\r\n");
	fprintf(stream, "1C\r\n");
	fprintf(stream, "and this is the second one\r\n");
	fprintf(stream, "3\r\n");
	fprintf(stream, "con\r\n");
	fprintf(stream, "8\r\n");
	fprintf(stream, "sequence\r\n");
	fprintf(stream, "0\r\n");
	fclose(stream);
	free(line);
}
*/
	//is this an HTTP GET request?
	if(strncmp(line, "GET", strlen("GET")) == 0){

		//is this a GET request for "/" ?
		if(strncmp(line, "GET / HTTP", strlen("GET / HTTP")) == 0){
			ret = fprintf(stream, "HTTP/1.1 200 OK\r\n");
			__CHK_RET();

			ret = fprintf(stream, "Content-Type: text/plain\r\n");
			__CHK_RET();

			ret = fprintf(stream, "Connnection: close\r\n");
			__CHK_RET();

			ret = fprintf(stream, "\r\n");
			__CHK_RET();

			ret = fprintf(stream, "%s\r\n", argv[2]);
			__CHK_RET();

			fclose(stream);
		}
		// if not for "/", return HTTP 404
		else{
			ret = fprintf(stream, "HTTP/1.1 404 NOT FOUND\r\n");
			__CHK_RET();

			ret = fprintf(stream, "Content-Type: text/plain\r\n");
			__CHK_RET();

			ret = fprintf(stream, "Connnection: close\r\n");
			__CHK_RET();

			ret = fprintf(stream, "\r\n");
			__CHK_RET();

			fclose(stream);
		}
	// if not HTTP GET, close connection
	}else{
		fclose(stream);
	}

	free(line);

    }  // end: main server loop


    return 0;
}
