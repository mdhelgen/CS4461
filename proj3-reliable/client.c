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

#define MAXBUFLEN 100

#define WINDOW_SIZE 3

int main(int argc, char *argv[])
{
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	socklen_t addr_len;
	struct sockaddr_storage their_addr;
	char buf[MAXBUFLEN];

	int cs_pass; 				//return value for checksum validation
	int packets_to_send = 0; 	//how many message packets remain to send to the server
	int packets_sent = 0; 		//how many total message packets we have sent
	int acks_outstanding = 0;	//the number of outstanding ACKs in the window
	int current_seq_no = 0; 	//the value of the next packet sequence number

	struct packet pkt;          //packet struct for manipulating packet data
	char* pkt_string;			//packets are encoded as strings before sending

	struct packet window[WINDOW_SIZE];
	int window_slot_full[WINDOW_SIZE];

	//initialize the window as having no contained packets
	for(int i = 0; i < WINDOW_SIZE; i++)
		window_slot_full[i] = 0;


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

	
	//send a SYN packet to the server to signal we want to open a connection

	pkt.syn = 1;
	pkt.ack = 0;
	pkt.fin = 0;
	pkt.seq_no = current_seq_no++;
	pkt.msg = '~';

	//encode the SYN packet as a string
	pkt_string = create_packet_string(pkt);

	//send the SYN to the server
	rv = unreliable_sendto(sockfd, pkt_string, strlen(pkt_string), 0, p->ai_addr, p->ai_addrlen);
	if(rv == -1){
		perror("client: unreliable_sendto");
	}
	free(pkt_string);

	//listen for an ACK
	bzero(buf, MAXBUFLEN);
	addr_len = sizeof(their_addr);
	recvfrom(sockfd, buf, MAXBUFLEN-1, 0, (struct sockaddr*) &their_addr, &addr_len);
	printf("RECV: %s\n", buf);
	struct packet resp;
	cs_pass = convert_to_packet(buf, &resp);

	if(resp.ack){
		printf("Received ACK for seq_no %d %s\n", resp.seq_no, cs_pass ? "" : "(checksum failed)");
	}

	packets_to_send = strlen(argv[2]);
	
	//TODO: this for loop will eventually have to change for the sliding window	
	//for(int i=0; i < strlen(argv[2]); i++)
	while(packets_sent < packets_to_send || acks_outstanding > 0)
	{
	//	printf("argv[2][%d] = %c\n", i, argv[2][i]);

		while(acks_outstanding < WINDOW_SIZE){

			if(packets_sent == packets_to_send)
				break;

			//construct the packet
			pkt.syn = 0;
			pkt.ack = 0;
			pkt.fin = 0;
			pkt.seq_no = current_seq_no++;
			pkt.msg = argv[2][packets_sent];

			int j = 0;
			while(window_slot_full[j] && j < WINDOW_SIZE)
				j++;

			//encode the packet as a string
			pkt_string = create_packet_string(pkt);
			printf("packet string:  %s (window slot = %d)\n", pkt_string, j);

			//copy the packet into the window
			memcpy(&window[j] , &pkt, sizeof(pkt));

			//mark the window as being full there
			window_slot_full[j] = pkt.seq_no;

			rv = unreliable_sendto(sockfd, pkt_string, strlen(pkt_string),
	   	               0, p->ai_addr, p->ai_addrlen);
			if(rv == -1){
				perror("client: unreliable_sendto");
			}

			packets_sent++;
			acks_outstanding++;

			free(pkt_string);
		}


		//listen for incoming packets
		fd_set readfds;
		struct timeval timeout;
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;
		FD_ZERO(&readfds);
		FD_SET(sockfd, &readfds);
		rv = select(sockfd+1, &readfds, NULL, NULL, &timeout);
		if(rv){
			bzero(buf, MAXBUFLEN);
			addr_len = sizeof(their_addr);
			recvfrom(sockfd, buf, MAXBUFLEN-1, 0, (struct sockaddr*) &their_addr, &addr_len);
			printf("RECV: %s\n", buf);
			cs_pass = convert_to_packet(buf, &resp);

			if(resp.ack){
				acks_outstanding--;

				for(int i = 0; i < WINDOW_SIZE; i++){
					if(window_slot_full[i] == resp.seq_no){
						printf("ACK received for seq no %d, window slot %d\n", resp.seq_no, i);
						window_slot_full[i] = 0;
						bzero(&window[i], sizeof(struct packet));
					}
					
				}


				printf("Received ACK for seq_no %d %s\n", resp.seq_no, cs_pass ? "" : "(checksum failed)");
			}
		}
	}

	//send our FIN
	pkt.syn = 0;
	pkt.ack = 0;
	pkt.fin = 1;
	pkt.seq_no = current_seq_no++;
	pkt.msg = '~';

	pkt_string = create_packet_string(pkt);

	rv = unreliable_sendto(sockfd, pkt_string, strlen(pkt_string), 0, p->ai_addr, p->ai_addrlen);
	if(rv == -1){
		perror("client: unreliable_sendto");	
	}


	bzero(buf, MAXBUFLEN);
	addr_len = sizeof(their_addr);
	recvfrom(sockfd, buf, MAXBUFLEN-1, 0, (struct sockaddr*) &their_addr, &addr_len);
	printf("RECV: %s\n", buf);
	cs_pass = convert_to_packet(buf, &resp);

	if(resp.ack){
		printf("Received ACK for seq_no %d %s\n", resp.seq_no, cs_pass ? "" : "(checksum failed)");
	}

	

	freeaddrinfo(servinfo);

	close(sockfd);

	return 0;
}
