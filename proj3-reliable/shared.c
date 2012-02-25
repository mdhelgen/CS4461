#include "unreliable.h"

/* Put any functions here which you want to be
   able to use by both client.c and server.c. */

struct packet{
	int syn;
	int fin;
	int ack;
	int seq_no;
	char msg;
	int checksum;
};

//packet format: 1_0_0_10_
//   <syn>_<ack>_<fin>_<seq>_<msg>_<checksum>
char* create_packet_string(struct packet input){
   char* target = malloc(12+ input.seq_no%10 + input.checksum%10);
   sprintf(target, "%d %d %d %d %c %d", input.syn, input.ack, input.fin, input.seq_no, input.msg, input.checksum);

	return target;
}

int calc_checksum(struct packet input){
	return input.syn + input.ack + input.fin + input.seq_no + input.msg;
}
