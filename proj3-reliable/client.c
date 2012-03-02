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
	int duplicateAck = 0;

	struct timeval current_tv;
	struct packet pkt;          //packet struct for manipulating packet data
	char* pkt_string;			//packets are encoded as strings before sending

	struct packet window[WINDOW_SIZE];  //holds packet data for packets waiting to be ACKd
	int window_slot_full[WINDOW_SIZE];  //holds the seq_no for the packet in that slot in the window, 0 if empty

	//initialize the window as having no contained packets
	for(int i = 0; i < WINDOW_SIZE; i++)
		window_slot_full[i] = 0;

	//arg check
	if (argc != 3) {
		fprintf(stderr,"usage: %s hostname message\n", argv[0]);
		exit(1);
	}

	//setup for getaddrinfo
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;

	//get info about the server
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

		fd_set readfds;
		struct timeval timeout;
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;
		FD_ZERO(&readfds);
		FD_SET(sockfd, &readfds);
	
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

	bzero(buf, MAXBUFLEN);
	addr_len = sizeof(their_addr);

	//wait for something to read
	while(select(sockfd+1, &readfds, NULL, NULL, &timeout) == 0){
		
		//resend the packet
		rv = unreliable_sendto(sockfd, pkt_string, strlen(pkt_string), 0, p->ai_addr, p->ai_addrlen);
		if(rv == -1){
			perror("client: unreliable_sendto");
		}

		//wait again

		//the file descriptor list was getting reset for some reason
		FD_ZERO(&readfds);
		FD_SET(sockfd, &readfds);
	
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;
	}

	free(pkt_string);

	//listen for an ACK to our SYN
	recvfrom(sockfd, buf, MAXBUFLEN-1, 0, (struct sockaddr*) &their_addr, &addr_len);

	printf("RECV: %s\n", buf);
	struct packet resp;

	//convert the received message into a packet struct
	//cs_pass is the result of the checksum validation
	cs_pass = convert_to_packet(buf, &resp);

	if(resp.ack){
		printf("Received ACK for seq_no %d %s\n", resp.seq_no, cs_pass ? "" : "(checksum failed)");
	}

	packets_to_send = strlen(argv[2]);


	//while we still have packets to send, or are waiting for acks, go into the send/receive loop
	while(packets_sent < packets_to_send || acks_outstanding > 0)
	{

		//sliding window -- if there is room in the window, send packets
		while(acks_outstanding < WINDOW_SIZE){

			//however, don't send packets if the message has all been sent, and we are waiting for acks only
			if(packets_sent == packets_to_send)
				break;

			//construct the packet
			pkt.syn = 0;
			pkt.ack = 0;
			pkt.fin = 0;
			pkt.seq_no = current_seq_no++;
			pkt.msg = argv[2][packets_sent];

			//find the next available open slot in the window
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

			//send the packer
			rv = unreliable_sendto(sockfd, pkt_string, strlen(pkt_string),
	   	               0, p->ai_addr, p->ai_addrlen);
			if(rv == -1){
				perror("client: unreliable_sendto");
			}

			gettimeofday(&(window[j].tv), NULL);	

			packets_sent++;
			acks_outstanding++;

			free(pkt_string);
		}
		
		//the file descriptor list was getting reset for some reason
		FD_ZERO(&readfds);
		FD_SET(sockfd, &readfds);
	
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;

		//listen for incoming packets
		rv = select(sockfd+1, &readfds, NULL, NULL, &timeout);

		
		gettimeofday(&current_tv, NULL);


		if(rv){
			bzero(buf, MAXBUFLEN);
			addr_len = sizeof(their_addr);

			//receive a packet
			recvfrom(sockfd, buf, MAXBUFLEN-1, 0, (struct sockaddr*) &their_addr, &addr_len);
			printf("RECV: %s\n", buf);

			//convert packet to a string
			//cs_pass is a boolean indicator of the checksum valiation
			cs_pass = convert_to_packet(buf, &resp);


			//if the response is an ACK
			if(resp.ack && cs_pass){

				int ack_matched = 0;

				//window_slot_full array contains the seq_no of the packet in that window slot
				for(int i = 0; i < WINDOW_SIZE; i++){

					//if this slot matches the ACK we just received
					if(window_slot_full[i] == resp.seq_no){
						
						printf("ACK received for seq no %d, window slot %d\n", resp.seq_no, i);
						ack_matched = 1;
						//empty the window slot
						window_slot_full[i] = 0;
						bzero(&window[i], sizeof(struct packet));
						acks_outstanding--;
					}


					//if this slot has a seq_no less than the ACK we just received
					//the old ack was lost and should be removed due to cumulative ACKs
					if(window_slot_full[i] < resp.seq_no && window_slot_full[i] > 0){
						
						printf("cumulative ACK for seq no %d, window slot %d\n", window_slot_full[i], i);
						ack_matched = 1;
						//empty the window slot
						window_slot_full[i] = 0;
						bzero(&window[i], sizeof(struct packet));
						acks_outstanding--;
					}
					
				}

				//if we got an ACK we weren't waiting for, it was a duplicate
				if(ack_matched == 0){
					printf("ACK with seq_no %d received, but is not outstanding!\n", resp.seq_no);
					duplicateAck++;

					//after 3 duplicates, resend everything in the window
					if(duplicateAck == 3){
						duplicateAck = 0;

						printf("too many duplicate acks received, resending all in window\n");
						for(int i = 0; i < WINDOW_SIZE; i++){
							if(window_slot_full[i]){
								printf("\t resend packet with seq_no %d\n", window_slot_full[i]);
								pkt_string = create_packet_string(window[i]);

								rv = unreliable_sendto(sockfd, pkt_string, strlen(pkt_string), 0, p->ai_addr, p->ai_addrlen);
								if(rv == -1){
									perror("client: unreliable_sendto");
								}
								
								
								gettimeofday(&(window[i].tv), NULL);

								free(pkt_string);
							}

						}
					}
				}
			}
		}
		//the select() call timed out, resend all packets	
		else{
			printf("client timeout on receiving\n");
			for(int i = 0; i < WINDOW_SIZE; i++){
				if(window_slot_full[i]){
					printf("\t resend packet with seq_no %d\n", window_slot_full[i]);
			
					pkt_string = create_packet_string(window[i]);

					rv = unreliable_sendto(sockfd, pkt_string, strlen(pkt_string), 0, p->ai_addr, p->ai_addrlen);
					if(rv == -1){
						perror("client: unreliable_sendto");
					}
					gettimeofday(&(window[i].tv), NULL);
					free(pkt_string);
				}

			}
		}

		for(int i = 0; i < WINDOW_SIZE; i++){
			if(window_slot_full[i]){
				if(current_tv.tv_sec - window[i].tv.tv_sec > 2)
					printf("packet with seq no %d has timed out\n", window[i].seq_no);


			}


		}
	}
	int fin_seq_no = current_seq_no++;

	
	//send our FIN
	pkt.syn = 0;
	pkt.ack = 0;
	pkt.fin = 1;
	pkt.seq_no = fin_seq_no;
	pkt.msg = '~';

	pkt_string = create_packet_string(pkt);

	rv = unreliable_sendto(sockfd, pkt_string, strlen(pkt_string), 0, p->ai_addr, p->ai_addrlen);
	if(rv == -1){
		perror("client: unreliable_sendto");	
	}

	timeout.tv_sec = 1;
	timeout.tv_usec = 0;

	//wait for a response
	
	int connection_finished = 0;

	while(!connection_finished){
		while(select(sockfd+1, &readfds, NULL, NULL, &timeout) == 0) {
		
			//resend the packet
			rv = unreliable_sendto(sockfd, pkt_string, strlen(pkt_string), 0, p->ai_addr, p->ai_addrlen);
			if(rv == -1){
				perror("client: unreliable_sendto");
			}

			//wait again
			//the file descriptor list was getting reset for some reason
			FD_ZERO(&readfds);
			FD_SET(sockfd, &readfds);
	

			timeout.tv_sec = 1;
			timeout.tv_usec = 0;
		}

		bzero(buf, MAXBUFLEN);
		addr_len = sizeof(their_addr);
		recvfrom(sockfd, buf, MAXBUFLEN-1, 0, (struct sockaddr*) &their_addr, &addr_len);
		printf("RECV: %s\n", buf);
		cs_pass = convert_to_packet(buf, &resp);
	
		if(resp.ack){
			printf("Received ACK for seq_no %d %s\n", resp.seq_no, cs_pass ? "" : "(checksum failed)");
		}
		if(resp.seq_no == fin_seq_no)
			connection_finished = 1;
	
	}


	freeaddrinfo(servinfo);

	close(sockfd);

	return 0;
}
