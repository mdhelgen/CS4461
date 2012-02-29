/*
** server.c -- a datagram sockets "server" demo
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


#define MAXBUFLEN 100

int main(void)
{
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int numbytes;
    struct sockaddr_storage their_addr;
    char buf[MAXBUFLEN];
    socklen_t addr_len;
    char s[INET6_ADDRSTRLEN];
    char receivedMsg[2048];
	int receivedBytes = 0;

	int expected_seq_no = 0;
	int seq_no_skipped = 0;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, SERVERPORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
			     p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "server: failed to bind socket\n");
        return 2;
    }

    freeaddrinfo(servinfo);

    while(1)
    {
		printf("Waiting for something to read...\n");

		//TODO: change recvfrom() to select() to prevent blocking, and implement timeout
		addr_len = sizeof their_addr;
		if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0,
				 (struct sockaddr *)&their_addr,
				 &addr_len)) == -1)
		{
		    perror("recvfrom");
		    exit(1);
		}
		buf[numbytes] = '\0';
		printf("Server received: %s\n", buf);

		struct packet pkt;
		bzero(&pkt, sizeof(pkt));
		int cs_pass = convert_to_packet(buf, &pkt);

		if(pkt.seq_no != expected_seq_no){
			
			seq_no_skipped = expected_seq_no - 1;

		}

		printf("received packet containing: %c %s\n", pkt.msg, cs_pass ? "" : "(checksum failed)"); 
		if(!(pkt.syn + pkt.ack + pkt.fin) && cs_pass){
			receivedMsg[receivedBytes++] = pkt.msg;
			receivedMsg[receivedBytes] = '\0';
		}

		if(pkt.fin && cs_pass){
			//TODO: close the connection cleanly
			printf("received FIN\n");
			printf("The complete message is: %s\n", receivedMsg);
		}
		
		//create the response packet
		struct packet resp_pkt;
		memcpy(&resp_pkt, &pkt, sizeof(pkt));

		resp_pkt.ack = 1;
		resp_pkt.syn = 0;
		resp_pkt.fin = 0;
		resp_pkt.msg = '~';
		if(seq_no_skipped){
		//	resp_pkt.seq_no = seq_no_skipped;

		}
		char* resp_str = create_packet_string(resp_pkt);

		printf("response packet: %s\n", resp_str);

		rv = unreliable_sendto(sockfd, resp_str, strlen(resp_str), 0, (struct sockaddr*) &their_addr, addr_len);
		if(rv == -1){
			perror("server: unreliable_sendto");
		}
		free(resp_str);

		//TODO: after receiving a packet, respond with an ACK

    }



    close(sockfd);

    return 0;
}

