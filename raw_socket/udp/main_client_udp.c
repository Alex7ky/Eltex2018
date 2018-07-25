#include "header_udp.h"

/*
* Создание udp клиента 	
*/
int main()
{
	int sock_fd;
	uint16_t total_size; 
	char buf[SIZE_UDP_PACKET];
	struct sockaddr_in remote;
	
	sock_fd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
	
	if (sock_fd == -1) {
		perror("socket create");
		exit(-1);
	}

	/* формирование udp сегмента */
	total_size = create_packet_udp(buf);
	
	bzero(&remote, sizeof(remote));
	
	remote.sin_family      = AF_INET;
	remote.sin_port        = htons(SERVER_PORT);
	remote.sin_addr.s_addr = inet_addr(D_ADDR);
	
	if (sendto(sock_fd, buf, total_size, 0, (struct sockaddr *)&remote,
		sizeof(remote)) < 0){
		perror("close_socket");
		exit(-1);
	} else {
		printf("sendto - ok\n");
	}	
		
	if (close(sock_fd) < 0) {
		perror("close_socket");
		exit(-1);
	} else {
		printf("close_socket - ok\n");
	}		 

	return 0;
}
