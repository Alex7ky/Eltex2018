#include "header_ip_udp.h"

/**
 * Создание ip и udp сегмента
 *
 * @param *buffer начало порядка байт, для записи ip и udp сегмента
 *
 * @retval total_size полный размер ip и udp сегмента
 */
uint16_t create_packet_ip_udp(char *buffer) {
	int i;
	IP_HDR ip_hdr;
	UDP_HDR udp_hdr;
	uint16_t total_size, ip_size, udp_size, udp_checksum;
	char *ptr, message[SIZE_MSG];
	
	strcpy(message, "Hello! (udp_header, ip_header)\n");
	
	total_size = sizeof(ip_hdr) + sizeof(udp_hdr) + strlen(message);
	ip_size    = sizeof(ip_hdr) / sizeof(uint32_t);
			
	ip_hdr.version  = 4;
	ip_hdr.ihl      = ip_size;
	ip_hdr.tos      = 0;
	ip_hdr.tot_len  = htons(total_size);
	ip_hdr.id       = 0;
	ip_hdr.frag_off = 0;
	ip_hdr.ttl      = 128;
	ip_hdr.protocol = IPPROTO_UDP;
	ip_hdr.check    = 0;
	ip_hdr.saddr    = inet_addr(S_ADDR);
	ip_hdr.daddr    = inet_addr(D_ADDR);
							
	udp_size = sizeof(udp_hdr) + strlen(message);
	udp_hdr.source_port = htons(CLIENT_PORT);
	udp_hdr.dest_port   = htons(SERVER_PORT);
	udp_hdr.udp_len     = htons(udp_size);
	udp_hdr.udp_sum     = 0;
	udp_checksum = 0;
	ptr = buffer;
		
	/* * Подсчет чексуммы udp сегмента * */
	bzero(buffer, SIZE_IP_UDP_PACKET);

	memcpy(ptr, &ip_hdr.saddr, sizeof(ip_hdr.saddr));  
	ptr          += sizeof(ip_hdr.saddr);
	udp_checksum += sizeof(ip_hdr.saddr);

	memcpy(ptr, &ip_hdr.daddr, sizeof(ip_hdr.daddr));
	ptr          += sizeof(ip_hdr.daddr);
	udp_checksum += sizeof(ip_hdr.daddr);
	ptr++;
	udp_checksum += 1;

	memcpy(ptr, &ip_hdr.protocol, sizeof(ip_hdr.protocol));
	ptr          += sizeof(ip_hdr.protocol);
	udp_checksum += sizeof(ip_hdr.protocol);

	memcpy(ptr, &udp_hdr.udp_len, sizeof(udp_hdr.udp_len));
	ptr          += sizeof(udp_hdr.udp_len);
	udp_checksum += sizeof(udp_hdr.udp_len);

	memcpy(ptr, &udp_hdr, sizeof(udp_hdr));
	ptr          += sizeof(udp_hdr);
	udp_checksum += sizeof(udp_hdr);
		
	for(i = 0; i < strlen(message); i++, ptr++)
		*ptr = message[i];
			
	udp_checksum += strlen(message);

	udp_hdr.udp_sum = checksum((uint16_t *)buffer, udp_checksum); 
	/* * * * * * * * * * * * * * * * * * * */					 
							 
	bzero(buffer, SIZE_IP_UDP_PACKET);

	ptr = buffer;

	memcpy(ptr, &ip_hdr, sizeof(ip_hdr)); 

	ptr += sizeof(ip_hdr);

	memcpy(ptr, &udp_hdr,sizeof(udp_hdr)); ptr += sizeof(udp_hdr);
	memcpy(ptr, message, strlen(message));
	
	return total_size;
}
