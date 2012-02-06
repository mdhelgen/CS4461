// Matt Helgen 
// CS4461 - Computer Networks

#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <arpa/inet.h>


//generic error check definition
#define __ERROR_CHECK(x,y) if(ret == y){ perror(x); exit(y);}

int bigEndian = 0;

void checkEndian()
{
	/* PART 1: Check endianness and print message about endianness here. */


	printf("Networks use big endian ordering\n");

	//if host to network byte order conversion changes the value, we are using little-endian
	//
	bigEndian = ((1<<30) == htonl(1<<30));

	if (bigEndian)
		printf("This host uses big endian (most significant byte first)\n");
	else
		printf("This host uses little endian (least significant byte first)\n");

}



int main(int argc, char *argv[])
{
	checkEndian();


	/* The "hints" structure lets us specify exactly what type of
	   address we want to get back from getaddrinfo(). */
	struct addrinfo hints;  // create struct
	memset(&hints, 0, sizeof(hints)); // fill struct with zeros

	// see the getaddrinfo() man page for descriptions of these variables:
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_family = AF_UNSPEC;
	/* Uncomment this if you only want to resolve the versions of IP that
	   your computer appears to support.  For this homework, leave this
	   commented out. */
	// hints.ai_flags = AI_ADDRCONFIG;


	struct addrinfo *answer;
	int error = getaddrinfo(argv[1], NULL, &hints, &answer);
	if(error != 0)
	{
		fprintf(stderr, "Error in getaddringo(): %s\n", gai_strerror(error));
		exit(EXIT_FAILURE);
	}
	/* PART 2 & 3: use information in the variable "answer". */

	//save the first element of the linked list
	struct addrinfo *answertemp = answer;


	//return values for getnameinfo()
	char host_buf[1025];
	char host_buf2[1025];
	char serv_buf[32];
	int ret;

	//iterate the list of addressinfo structs
	while(answer->ai_next != NULL){

		//display ip version
		if(answer->ai_family == AF_INET)
			printf("ipv4: ");
		if(answer->ai_family == AF_INET6)
			printf("ipv6:  ");

		//get the numerical address
		ret = getnameinfo(answer->ai_addr, answer->ai_addrlen, host_buf, 1024, serv_buf, 32, NI_NUMERICHOST);

		printf(" %s\n",host_buf);

		//next addressinfo entry
		answer = answer->ai_next;
	}

	//return to the beginning of the linked list
	answer = answertemp;

	//iterate the linked list
	while(answer->ai_next != NULL){

		//display ip version
		if(answer->ai_family == AF_INET)
			printf("ipv4: ");
		if(answer->ai_family == AF_INET6)
			printf("ipv6:  ");

		//get the numerical address in host_buf
		ret = getnameinfo(answer->ai_addr, answer->ai_addrlen, host_buf, 1024, serv_buf, 32, NI_NUMERICHOST);
		//get the host name in host_buf2
		ret = getnameinfo(answer->ai_addr, answer->ai_addrlen, host_buf2, 1024, serv_buf, 32, NI_NAMEREQD);

		printf(" %s reverse DNS lookup: %s\n",host_buf, host_buf2);
	
		//next addressinfo entry
		answer = answer->ai_next;

	}

	//tidy up
	freeaddrinfo(answer);
	return 0;



}
