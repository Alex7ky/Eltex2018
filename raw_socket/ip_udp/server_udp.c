#include "header_ip_udp.h"

/*
* Создание udp сервера 
*/
int main(int argc, char**argv)
{
	int sock_fd;
	struct sockaddr_in servaddr, cliaddr;

	socklen_t len;
	char mesg[SIZE_MSG];

	sock_fd = socket(AF_INET, SOCK_DGRAM, 0);

	if (sock_fd < 0) {
		perror("create server socket");
		exit(-1);
	}

	memset(&servaddr, 0, sizeof(servaddr));

	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = INADDR_ANY;
	servaddr.sin_port = htons(SERVER_PORT);
	
	bind(sock_fd, (struct sockaddr *)&servaddr, sizeof(servaddr));

	for (;;) {
		len = sizeof(cliaddr);
		memset(mesg, 0, sizeof(char) * SIZE_MSG);

		if (recvfrom(sock_fd, mesg, SIZE_MSG, 0, 
			        (struct sockaddr *)&cliaddr, &len) < 0) {
			close(sock_fd);
			perror("server recv err");
			exit(-1);
		}

		printf("recvfrom: %s\n", mesg);
	}

	return 0;
}

