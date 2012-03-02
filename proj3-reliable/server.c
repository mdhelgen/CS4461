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
    char receivedMsg[2048];
	int receivedBytes = 0;

	int cumulative_ack = 0;
	int expected_seq_no = 0;

	int no_connection = 1;

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

		//construct the packet
		struct packet pkt;
		bzero(&pkt, sizeof(pkt));
		int cs_pass = convert_to_packet(buf, &pkt);

		//sometimes a corrupted packet will mess up the conversion to a struct
		//this results in a seq no of 0, and sometimes can affect the checksum as well
		//this happens because the corruption of a number to a character can mess up fscanf
		if(!pkt.syn && pkt.seq_no == 0){

			//just force the checksum to fail
			cs_pass = 0;
		}

		//open the connection
		if(pkt.syn && cs_pass && no_connection){
			expected_seq_no = 0;
			receivedBytes = 0;
			bzero(receivedMsg, 2048);
			no_connection = 0;

		}

		//cumulative ack stuff
		if(pkt.seq_no == expected_seq_no && cs_pass){

			cumulative_ack = pkt.seq_no;
			expected_seq_no++;

		}

		printf("received packet containing: %c %s\n", pkt.msg, cs_pass ? "" : "(checksum failed)"); 

		//message packet
		if(!(pkt.syn + pkt.ack + pkt.fin) && cs_pass){
			
			//receivedMsg[receivedBytes] = ' ';	
			if(pkt.seq_no > 0)
				receivedMsg[pkt.seq_no - 1] = pkt.msg;

			printf("msg: %s\t%d = %c\n", receivedMsg, pkt.seq_no-1, pkt.msg);
		}

		//close connection
		if(pkt.fin && cs_pass && !no_connection){
			//TODO: close the connection cleanly
			printf("received FIN\n");
			printf("**********************************************\n");
			printf("The complete message is: %s\n", receivedMsg);
			printf("**********************************************\n");
			no_connection = 1;
			expected_seq_no = 0;
			bzero(receivedMsg, 2048);
			printf("\n");

		}

		//checksum failure
		if(!cs_pass){
					
		}
		
		//create the response packet
		struct packet resp_pkt;
		memcpy(&resp_pkt, &pkt, sizeof(pkt));

		resp_pkt.ack = 1;
		resp_pkt.syn = 0;
		resp_pkt.fin = 0;
		resp_pkt.msg = '~';
		resp_pkt.seq_no = cumulative_ack;


		char* resp_str = create_packet_string(resp_pkt);

		printf("response packet: %s\n", resp_str);

		rv = unreliable_sendto(sockfd, resp_str, strlen(resp_str), 0, (struct sockaddr*) &their_addr, addr_len);
		if(rv == -1){
			perror("server: unreliable_sendto");
		}
		free(resp_str);


    }



    close(sockfd);

    return 0;
}

