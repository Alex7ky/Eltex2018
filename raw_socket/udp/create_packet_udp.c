#include "header_udp.h"

/**
 * Создание udp сегмента
 *
 * @param *buffer начало порядка байт, для записи udp сегмента
 *
 * @retval total_size полный размер udp сегмента
 */
uint16_t create_packet_udp(char *buffer) {
		
	UDP_HDR udp_hdr;
	uint16_t total_size, udp_size;
	char *ptr, message[SIZE_MSG];
	
	strcpy(message, "Hello! (udp_header)\n");
	
	total_size = sizeof(udp_hdr) + strlen(message);
				
	udp_size = sizeof(udp_hdr) + strlen(message);

	udp_hdr.source_port = htons(CLIENT_PORT);
	udp_hdr.dest_port   = htons(SERVER_PORT);
	udp_hdr.udp_len     = htons(udp_size);
	udp_hdr.udp_sum     = 0;
			 
	bzero(buffer, SIZE_UDP_PACKET);

	ptr = buffer ;

	memcpy(ptr, &udp_hdr, sizeof(udp_hdr)); ptr += sizeof(udp_hdr);
	memcpy(ptr, message, strlen(message));
	
	return total_size;
}
