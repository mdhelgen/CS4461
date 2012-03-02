/* unreliable_sendto() by Scott Kuhl
 *
 * A method that wraps around sendto() which randomly drops, corrupts, and
 * changes order of packets.  Its return values will ALWAYS indicate that
 * everything is sent properly.  It will, however, print messages about
 * what is going on.
 *
 * The parameters and return value for unreliable_sendto() are the same
 * as those to sendto().
 *
 */
#include "unreliable.h"

// 0.05 means that the event will happen about 5% of the time.
#define DROP_PERCENTAGE 0.2 
#define CORRUPT_PERCENTAGE 0.2 
#define ORDER_PERCENTAGE 0.2


// set DEBUG to 1 to just call sendto() directly (should be the same
// as setting all of the percentages above to 0).
#define DEBUG 0



#if (DEBUG==1)
ssize_t unreliable_sendto(int sockfd, const void *buf, size_t len,
                          int flags, const struct sockaddr *dest_addr,
                          socklen_t addrlen)
{
#warning "unreliable_sendto() is now using sendto() directly!"
	return sendto(sockfd, buf, len, flags, dest_addr, addrlen);
}
#else


// place to hold data for out-of-order packets:
static int hold = 0;
static int hold_sockfd;
static void *hold_buf;
static size_t hold_len;
static int hold_flags;
static struct sockaddr *hold_dest_addr;
static socklen_t hold_addrlen;


// send previously held packet if there is one.
void send_held_packet()
{
	if(hold == 0)  // nothing to do.
		return;
	hold = 0;

	fprintf(stderr, "SENT HELD: Sending packet held earlier. Contents=");
	if(write(2, hold_buf, hold_len) == -1)
	{
		printf("\nFailed to write() packet to stderr.\n");
		exit(EXIT_FAILURE);
	}

	fprintf(stderr, "\n");

	sendto(hold_sockfd, hold_buf, hold_len,
	       hold_flags, hold_dest_addr, hold_addrlen);
	free(hold_buf);
	free(hold_dest_addr);
}


static int needs_seed = 1;

ssize_t unreliable_sendto(int sockfd, const void *buf, size_t len,
                          int flags, const struct sockaddr *dest_addr,
                          socklen_t addrlen)
{
	if(needs_seed) // run once to seed random number generator.
	{
		needs_seed = 0;
		int seed = time(NULL) + getpid() + getppid();
		srand48(seed);
	}

	fprintf(stderr, "SEND: ");
	if(write(2, buf, len) == -1)
	{
		printf("\nFailed to write() packet to stderr.\n");
		exit(EXIT_FAILURE);
	}
	fprintf(stderr, " ");

	if(drand48() < DROP_PERCENTAGE)
	{
		fprintf(stderr, "(DROPPED!)\n");
		return len;
	}

	if(drand48() < CORRUPT_PERCENTAGE)
	{
		fprintf(stderr, "(CORRUPTED!)\n");

		// Corrupt a copy of the data:
		char corrupted[len];
		memcpy(corrupted, buf, len);

		// pick a byte to corrupt
		int byte_to_corrupt = (int) (drand48() * len);
		int bit_to_corrupt = (int) (drand48() * 8);
		char* ptr = corrupted + byte_to_corrupt;
		if( (*ptr) && (0x01 << bit_to_corrupt) == 0) // bit is currently 0
			(*ptr) = (*ptr) || (0x01 << bit_to_corrupt);
		else
			(*ptr) = (*ptr) ^ (0x01 << bit_to_corrupt);

		// verify our corruption:
		if(memcmp(corrupted, buf, len) == 0)
		{
			printf("Corruption routine failed to corrupt. %d %d",
			       byte_to_corrupt, bit_to_corrupt);
			printf("Email Scott.\n");
			exit(1);
		}

		// send the corrupted packet
		int retval = sendto(sockfd, corrupted, len, flags, dest_addr, addrlen);
		send_held_packet();
		return retval;
	}

	// Hold on to packet until next sent packet (if we aren't already holding
	// a packet).
	if(drand48() < ORDER_PERCENTAGE && hold == 0)
	{
		hold= 1;
		hold_sockfd = sockfd;
		hold_buf = malloc(sizeof(char)*len);
		memcpy(hold_buf, buf, len);
		hold_len = len;
		hold_flags = flags;
		hold_dest_addr = malloc(sizeof(struct sockaddr));
		memcpy(hold_dest_addr, dest_addr, sizeof(struct sockaddr));
		hold_addrlen = addrlen;

		fprintf(stderr, "(WILL SEND LATER!)\n");
		return len;  	// pretend that we sent the packet successfully
	}

	// If we made it this far, we will send packet normally:
	int retval = sendto(sockfd, buf, len, flags, dest_addr, addrlen);
	fprintf(stderr, "(SUCCESS!)\n");
	send_held_packet();
	return retval;
}

#endif
