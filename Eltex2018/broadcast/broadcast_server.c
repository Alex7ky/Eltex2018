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
	char               buf[MSG_MAXLEN];
    int                broadcastEnable;

	sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if (sockfd == -1) {
        	perror("socket");
        	exit(-1);
	}

	broadcastEnable = 1;

	if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable)) == -1) {
		perror("setsockopt");
        exit(-1);
	}

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	inet_pton(AF_INET, SRV_ADDR, &(servaddr.sin_addr));
	servaddr.sin_port = htons(SRV_PORT);

	strcpy(buf, "Hello");

	while (1) {
        	printf("send message to client: %s\n", buf);
        	sleep(2);
        	sendto(sockfd, buf, strlen(buf)+1, 0, (struct sockaddr *)&servaddr, sizeof(servaddr));
	}

	close(sockfd);
	exit(0);
}
