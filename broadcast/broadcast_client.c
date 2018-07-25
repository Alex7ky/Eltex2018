#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define SRV_PORT    5773
#define SRV_ADDR    "127.0.0.255"
#define MSG_MAXLEN  256

int main()
{
	int                sockfd;
	struct sockaddr_in servaddr;
	socklen_t          addr_size; 
	char               msg[MSG_MAXLEN];
	int                msg_size;
	int                broadcastEnable = 1;

	sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if (sockfd == -1) {
			perror("socket");
			exit(-1);
	}

	bzero(&servaddr, sizeof(servaddr));

	servaddr.sin_family = AF_INET;
	inet_pton(AF_INET, SRV_ADDR, &(servaddr.sin_addr));
	servaddr.sin_port = htons(SRV_PORT);

	if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
		perror("bind");
		exit(-1);
	}

	addr_size = sizeof(servaddr);

	while (1) {
		printf("recvfrom: \n");
		msg_size = recvfrom(sockfd, msg, MSG_MAXLEN, 0, (struct sockaddr *)&servaddr, &addr_size);
		if (msg_size == -1) {
			perror("recvfrom");
			exit(-1);
		} else
			printf("%s\n", msg);
	}

	close(sockfd);
	exit(0);
}
