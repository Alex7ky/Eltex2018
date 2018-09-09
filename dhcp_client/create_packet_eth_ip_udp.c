#include "header_ip_udp.h"

/**
 * Создание ethernet, ip, udp сегмента
 *
 * @param *buffer начало порядка байт, для записи ethernet, ip, udp сегмента
 *
 * @retval total_size полный размер ethernet, ip, udp сегмента
 */
uint16_t create_packet_eth_ip_udp(char *buffer) {
	IP_HDR ip_hdr;
	UDP_HDR udp_hdr;
	BOOTP_HDR bootp_hdr;
	uint16_t total_size, ip_size, udp_size, udp_checksum;
	char *ptr;
	
	total_size = sizeof(ip_hdr) + sizeof(udp_hdr) + sizeof(bootp_hdr);
	ip_size    = sizeof(ip_hdr) / sizeof(uint32_t);
			
	ip_hdr.version  = 4;
	ip_hdr.ihl      = ip_size;
	ip_hdr.tos      = 0;
	ip_hdr.tot_len  = htons(total_size);
	ip_hdr.id       = 0;
	ip_hdr.frag_off = 0;
	ip_hdr.ttl      = 64;
	ip_hdr.protocol = IPPROTO_UDP;
	ip_hdr.check    = 0x00;
	ip_hdr.saddr    = inet_addr(SOUR_ADDR);
	ip_hdr.daddr    = inet_addr(DEST_ADDR);				
	
	/* * Подсчет чексуммы ip сегмента * */
	ip_hdr.check = checksum((uint16_t *)&ip_hdr, ip_hdr.ihl * 4);

	bootp_hdr.op        = 1;
	bootp_hdr.htype     = 0x01;
	bootp_hdr.hlen      = 6;
	bootp_hdr.hops      = 0;
	bootp_hdr.xid       = 0; 
	bootp_hdr.secs      = 0;
	bootp_hdr.flags     = 0;
	bootp_hdr.client_ip = 0;
	bootp_hdr.your_ip   = 0;
	bootp_hdr.server_ip = 0;
	bootp_hdr.relay_ip  = 0;

	for (int i = 0; i < 16; i++) 
		bootp_hdr.hw_addr[i]  = 0;

	bootp_hdr.hw_addr[0] = SR_MAC0;
	bootp_hdr.hw_addr[1] = SR_MAC1;
	bootp_hdr.hw_addr[2] = SR_MAC2; 
	bootp_hdr.hw_addr[3] = SR_MAC3; 
	bootp_hdr.hw_addr[4] = SR_MAC4;
	bootp_hdr.hw_addr[5] = SR_MAC5;

	for (int i = 0; i < 64; i++) 
		bootp_hdr.serv_name[i] = 0;
	
	for (int i = 0; i < 128; i++)
		bootp_hdr.boot_file[i] = 0;

	for (int i = 0; i < 64; i++)	
		bootp_hdr.exten[i] = 0;

	// Magick cookie
	bootp_hdr.exten[0] = 0x63;
	bootp_hdr.exten[1] = 0x82;
	bootp_hdr.exten[2] = 0x53;
	bootp_hdr.exten[3] = 0x63;

	// DHCP Message type
	bootp_hdr.exten[4] = 0x35;
	bootp_hdr.exten[5] = 0x01;
	bootp_hdr.exten[6] = 0x01;

	// Client identifier
	bootp_hdr.exten[7] = 0x3d;
	bootp_hdr.exten[8] = 0x07;
	bootp_hdr.exten[9] = 0x01;
	
	bootp_hdr.exten[10] = SR_MAC0;
	bootp_hdr.exten[11] = SR_MAC1;
	bootp_hdr.exten[12] = SR_MAC2; 
	bootp_hdr.exten[13] = SR_MAC3; 
	bootp_hdr.exten[14] = SR_MAC4;
	bootp_hdr.exten[15] = SR_MAC5;

	// Maximum DHCP Message Size
	bootp_hdr.exten[16] = 0x39;
	bootp_hdr.exten[17] = 0x02;
	bootp_hdr.exten[18] = 0x05;
	bootp_hdr.exten[19] = 0xdc;

	// END hdr
	bootp_hdr.exten[62] = 0xff;
	bootp_hdr.exten[63] = 0x00;
	
	udp_size = sizeof(udp_hdr) + sizeof(bootp_hdr);
	udp_hdr.source_port = htons(SOUR_PORT);
	udp_hdr.dest_port   = htons(DEST_PORT);
	udp_hdr.udp_len     = htons(udp_size);
	udp_hdr.udp_sum     = 0;

	udp_checksum = 0;

	ptr = buffer;
		
	/* * Подсчет чексуммы udp сегмента * */
	bzero(buffer, SIZE_IP_UDP_PACKET);

	memcpy(ptr, &ip_hdr.saddr, sizeof(ip_hdr.saddr));  
	ptr         += sizeof(ip_hdr.saddr);
	udp_checksum += sizeof(ip_hdr.saddr);

	memcpy(ptr, &ip_hdr.daddr, sizeof(ip_hdr.daddr));
	ptr         += sizeof(ip_hdr.daddr);
	udp_checksum += sizeof(ip_hdr.daddr);
	ptr++;
	udp_checksum += 1;

	memcpy(ptr, &ip_hdr.protocol, sizeof(ip_hdr.protocol));
	ptr += sizeof(ip_hdr.protocol);
	udp_checksum += sizeof(ip_hdr.protocol);

	memcpy(ptr, &udp_hdr.udp_len, sizeof(udp_hdr.udp_len));
	ptr += sizeof(udp_hdr.udp_len);
	udp_checksum += sizeof(udp_hdr.udp_len);

	memcpy(ptr, &udp_hdr, sizeof(udp_hdr));
	ptr += sizeof(udp_hdr);
	udp_checksum += sizeof(udp_hdr);
		
	memcpy(ptr, &bootp_hdr, sizeof(bootp_hdr));
	ptr += sizeof(bootp_hdr);
	udp_checksum += sizeof(bootp_hdr);

	udp_hdr.udp_sum = checksum((uint16_t *)buffer, udp_checksum); 
	/* * * * * * * * * * * * * * * * * * * */					 
							 
	bzero(buffer, SIZE_IP_UDP_PACKET);

	ptr = buffer;

	memcpy(ptr, &ip_hdr, sizeof(ip_hdr));

	ptr += sizeof(ip_hdr);

	memcpy(ptr, &udp_hdr,sizeof(udp_hdr)); ptr += sizeof(udp_hdr);
	memcpy(ptr, &bootp_hdr, sizeof(bootp_hdr));
	
	//printf("SIZE sizeof(bootp_hdr) = %ld \n", sizeof(bootp_hdr));
	//printf("SIZE sizeof(ip_hdr) = %ld \n", sizeof(ip_hdr));
	//printf("SIZE sizeof(udp_hdr) = %ld \n", sizeof(udp_hdr));

	return total_size;
}
