#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define CLT_PORT    4337
#define GRP_ADDR    "239.0.0.11"
#define MSG_MAXLEN  256

int main()
{
	int                sockfd;
	struct sockaddr_in grpaddr;
	char               buf[MSG_MAXLEN];
	int                reuse;

	sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if (sockfd == -1){
		perror("socket");
		exit(-1);
	}

	reuse = 1;

	if (setsockopt(sockfd, IPPROTO_IP, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0){
		perror("setsockopt");
		exit(1);
	}

	bzero(&grpaddr, sizeof(grpaddr));
	grpaddr.sin_family = AF_INET;
	inet_pton(AF_INET, GRP_ADDR, &(grpaddr.sin_addr));
	grpaddr.sin_port = htons(CLT_PORT);

	strcpy(buf, "Hello");

	while (1) {
		printf("send message to client: %s\n", buf);
		sleep(2);
		sendto(sockfd, buf, strlen(buf) + 1, 0, (struct sockaddr *)&grpaddr, sizeof(grpaddr));
	}

	close(sockfd);
	exit(0);
}