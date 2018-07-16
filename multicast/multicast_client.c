#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define CLT_PORT    4337
#define CLT_ADDR    "127.0.0.1"
#define GRP_ADDR    "239.0.0.11"
#define MSG_MAXLEN  256

int main()
{
	int                sockfd;
	struct sockaddr_in servaddr, cliaddr;
	char               msg[MSG_MAXLEN];
	socklen_t          addr_size; 
	int                msg_size;
	struct ip_mreq mreq;

	sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if (sockfd == -1) {
		perror("socket");
		exit(-1);
	}

    mreq.imr_multiaddr.s_addr = inet_addr(GRP_ADDR);
    mreq.imr_interface.s_addr = inet_addr(CLT_ADDR);
    //mreq.imr_interface.s_addr = htonl(INADDR_ANY);

	if (setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
		perror("setsockopt");
		exit(1);
	}

	addr_size = sizeof(cliaddr);
	bzero(&cliaddr, sizeof(cliaddr));
	cliaddr.sin_family = AF_INET;
	inet_pton(AF_INET, CLT_ADDR, &(cliaddr.sin_addr));
	cliaddr.sin_port = htons(CLT_PORT);

	if (bind(sockfd, (struct sockaddr *)&cliaddr, addr_size) == -1) {
		perror("bind");
		exit(-1);
	}

	while (1) {
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
