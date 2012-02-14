// Matt Helgen
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

    // check return codes
    int ret = 0;
    // mark when we have received a blank line in the http response
    int blank = 0; 
    // mark whether or not we have seen a Transfer-Encoding: chunked header
    int chunked = 0;
    

    int len;

	//check arguments
	if(argc < 3){
		printf("\tUsage: ./client <hostname> <portnum>\n");
		exit(1);	
	}

    /* Setup socket. */
    int sock = common_createSocket(argv[1], argv[2]);
    FILE *stream = common_getStream(sock);


	
	printf("##### CLIENT IS SENDING THE FOLLOWING TO SERVER:\n");
	printf("GET / HTTP/1.1\n");
	printf("Host: %s\n", argv[1]);
	printf("Connection: close\n\n");


	//write the http request to the socket stream
	ret = fprintf(stream, "GET / HTTP/1.0\r\n");
	if (ret < 0){
		perror("@@@@@ error occurred when writing to socket");
		exit(EXIT_FAILURE);
	}	

	ret = fprintf(stream, "Host: %s\r\n", argv[1]);
	if (ret < 0){
		perror("@@@@@ error occurred when writing to socket");
		exit(EXIT_FAILURE);
	}	


	ret = fprintf(stream, "Connection: close\r\n\r\n");
	if (ret < 0){
		perror("@@@@@ error occurred when writing to socket");
		exit(EXIT_FAILURE);
	}	

	printf("##### CLIENT RECEIVED THE FOLLOWING FROM SERVER:\n");

    while(1) // while there is data to read
    {
	char *string = NULL;
	if (chunked == 0 || blank == 0)	
		len = common_getLine(&string, stream);
	if (chunked == 1 && blank == 1){
		
		//the next line will indicate how much data is in the chunk
		common_getLine(&string, stream);
		int chunkSize = strtol(string, NULL, 16);
	
		printf("##### Next chunk is %d bytes\n", chunkSize);

		if(chunkSize == 0){
			chunked = 0;
			continue;
		}
		
		
		//free the space allocated by common_getLine()
		free(string);


		//allocate space for the chunk + null byte
		string = (char*) malloc(chunkSize + 1);

		//read the chunk into the buffer
		ret = fread(string, sizeof(char), chunkSize, stream);
		
		//check to see if we actually read as much data as expected
		if(ret != chunkSize)
		{
			printf("@@@@@ Error occurred when attempting to read from socket\n");
			printf("@@@@@\t Expected %d bytes, got %d bytes instead\n", chunkSize, ret);
			exit(EXIT_FAILURE);
		}

		//null terminate the buffer
		string[chunkSize] = '\0';

		//advance the stream past the \r\n sequence which was inserted to the chunk data
		fseek(stream, 2, SEEK_CUR);		

		//string now contains the contents of the chunk
	}



	//printf("line length: %d (%d)\n",len, strcmp(string, "\r\n"));	


	if (len == -1){
		fclose(stream);
		free(string);
		printf("##### Connection closed\n");
		exit(EXIT_SUCCESS);
	}

	if (blank == 0 && !strncmp(string, "Transfer-Encoding: chunked\r\n", strlen("Transfer-Encoding: chunked\r\n"))){
		printf("##### Response is using chunked encoding\n");
		chunked = 1;

	}
	// TODO: Process the line that we have read.
	if (strcmp(string, "\r\n") == 0){
		printf("##### Just read blank line\n");
		blank++;
	}

	printf(string);

	free(string);

    } // end infinite loop

}
