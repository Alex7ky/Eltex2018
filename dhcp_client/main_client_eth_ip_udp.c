#include "header_ip_udp.h"

#define ETH_HEADER_LEN 14

#define ARPHRD_ETHER 1

/* * Сетевой интерфейс(ip a) * */
#define DEFAULT_IF	"wlp3s0b1"

/*
* Создание udp клиента 	
*/
int main()
{
	int sock_fd;
	int ifindex;
	struct sockaddr_ll socket_address;
	void *buffer = (void *)calloc(ETH_FRAME_SIZE, 1);
	char *data = (char *)buffer + ETH_HEADER_LEN;
	int16_t size_packet;
	
	unsigned char src_mac[6]  = {SR_MAC0, SR_MAC1, SR_MAC2, 
	                             SR_MAC3, SR_MAC4, SR_MAC5};
	unsigned char dest_mac[6] = {DS_MAC0, DS_MAC1, DS_MAC2, 
	                             DS_MAC3, DS_MAC4, DS_MAC5};
	
	sock_fd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	
	if (sock_fd < 0) {
		perror("create socket");
		exit(-1);
	}

	/* Получаем номер сетевого интерфейса */
	ifindex = if_nametoindex(DEFAULT_IF);

	if (ifindex == 0) {
		perror("if_nametoindex");
		exit(-1);
	}

	socket_address.sll_family   = AF_INET;	
	socket_address.sll_protocol = htons(ETH_P_IP);
	socket_address.sll_ifindex  = ifindex;
	socket_address.sll_hatype   = ARPHRD_ETHER;
	socket_address.sll_pkttype  = PACKET_HOST;
	socket_address.sll_halen    = ETH_ALEN;

	/* * Destination MAC * */
	socket_address.sll_addr[0] = dest_mac[0];
	socket_address.sll_addr[1] = dest_mac[1];
	socket_address.sll_addr[2] = dest_mac[2];
	socket_address.sll_addr[3] = dest_mac[3];
	socket_address.sll_addr[4] = dest_mac[4];
	socket_address.sll_addr[5] = dest_mac[5];
	
	/* * MAC - end * */
	socket_address.sll_addr[6]  = 0x00;
	socket_address.sll_addr[7]  = 0x00;

	memcpy((void *)buffer, (void *)dest_mac, ETH_ALEN);
	memcpy((void *)(buffer + ETH_ALEN), (void *)src_mac, ETH_ALEN);
	
	char ether_frame[2];

	ether_frame[0] = ETH_P_IP / 256;
	ether_frame[1] = ETH_P_IP % 256;
	
	memcpy((void *)(buffer + ETH_ALEN + ETH_ALEN), (void *)ether_frame, 2);  
	
	size_packet = create_packet_eth_ip_udp(data);

	printf("data len = %d\n", size_packet + ETH_HEADER_LEN);

	size_packet += ETH_HEADER_LEN;

	if (sendto(sock_fd, buffer, size_packet, 0, 
		(struct sockaddr *) &socket_address, sizeof(socket_address)) < 0){
		perror("sendto client");
		exit(-1);
	} else {
		printf("sendto - ok\n");
	}	
		
	if (close(sock_fd) < 0) {
		perror("close sock");
		exit(-1);
	} else {
		printf("close sock - ok\n");
	}		 

	return 0;
}
