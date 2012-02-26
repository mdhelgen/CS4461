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


/* int calc_checksum(struct packet)
 *
 * Calculate the checksum of a packet, by adding up the values it contains
 *
 */
int calc_checksum(struct packet input){

	return (input.syn << 2) + (input.ack << 3) + (input.fin << 4) + input.seq_no + input.msg;

}

/* char* create_packet_string(struct packet)
 * 
 *   Creates a string representation of a packet.
 *   The packet is formatted in the following way:
 *      <syn> <ack> <fin> <seq_no> <msg> <checksum>
 *   
 *   The checksum field is automatically calculated using the packet information.
 *
 *   This method allocates space for the string it returns.
 */
char* create_packet_string(struct packet input){

	input.checksum = calc_checksum(input);
	char* target = malloc(13+ input.seq_no%10 + input.checksum%10);
	sprintf(target, "%d %d %d %d %c %d\0", input.syn, input.ack, input.fin, input.seq_no, input.msg, input.checksum);

	return target;
}

/* int convert_to_packet(char*, struct packet)
 *
 *  Converts a string representation of a packet to a machine readable struct.
 *
 *  Returns 1 if the provided checksum matches the calculated checksum, 0 otherwise.
 */
int convert_to_packet(char* pkt_string_input, struct packet* output){
	sscanf(pkt_string_input, "%d %d %d %d %c %d", &(output->syn), &(output->ack), &(output->fin), &(output->seq_no), &(output->msg), &(output->checksum));
	
	return (output->checksum == calc_checksum(*output));
}

