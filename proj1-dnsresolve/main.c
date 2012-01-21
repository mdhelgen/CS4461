// YOUR NAME HERE
// CS4461 - Computer Networks

#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <arpa/inet.h>


void checkEndian()
{
	/* PART 1: Check endianness and print message about endianness here. */

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
	int error = getaddrinfo("google.com", NULL, &hints, &answer);
	if(error != 0)
	{
		fprintf(stderr, "Error in getaddringo(): %s\n", gai_strerror(error));
		exit(EXIT_FAILURE);
	}
	/* PART 2 & 3: use information in the variable "answer". */


	freeaddrinfo(answer);
	return 0;
}
