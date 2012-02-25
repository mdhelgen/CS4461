#include "unreliable.h"

/* Put the prototypes here for any functions here which you want to be able
   to use by both client.c and server.c. */

struct packet{
  int syn;
  int fin;
  int ack;
  int seq_no;
  char msg;
  int checksum;
};

// the UDP port users will be connecting to on server
#define SERVERPORT "29392"

char* create_packet_string(struct packet);
int calc_checksum(struct packet);
int convert_to_packet(char*, struct packet*);
