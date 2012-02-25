/*
** client.c
** From http://www.beej.us/guide/bgnet/output/html/singlepage/bgnet.html
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "unreliable.h"
#include "shared.h"



int main(int argc, char *argv[])
{
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;

	if (argc != 3) {
		fprintf(stderr,"usage: %s hostname message\n", argv[0]);
		exit(1);
	}

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;

	if ((rv = getaddrinfo(argv[1], SERVERPORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and make a socket
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
		                     p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}
		break;
	}

	if (p == NULL) {
		fprintf(stderr, "client: failed to bind socket\n");
		return 2;
	}

	int current_seq_no = 0;
	
	//send a SYN packet to the server to signal we want to open a connection
	struct packet pkt;

/*
	pkt.syn = 1;
	pkt.ack = 0;
	pkt.fin = 0;
	pkt.seq_no = current_seq_no++;
	pkt.msg = ':
*/
	for(int i=0; i < strlen(argv[2]); i++)
	{
	//	printf("argv[2][%d] = %c\n", i, argv[2][i]);
	
		//construct the packet
		pkt.syn = 0;
		pkt.ack = 0;
		pkt.fin = 0;
		pkt.seq_no = i;
		pkt.msg = argv[2][i];

		//encode the packet as a string
		char* pkt_string = create_packet_string(pkt);
		printf("packet string:  %s\n", pkt_string);

		rv = unreliable_sendto(sockfd, pkt_string, strlen(pkt_string),
	                  0, p->ai_addr, p->ai_addrlen);
		if(rv == -1){
			perror("client: unreliable_sendto");
		}

		free(pkt_string);
	}


	freeaddrinfo(servinfo);

	close(sockfd);

	return 0;
}
