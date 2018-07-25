#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#define BUFFER_SIZE 1024

void reverse_string(char *input, char *output);

int main(int argc, char *argv[]) 
{
	if (argc != 3) {
		fprintf(stderr, "[ERROR #1] Non-worm number of arguments\n");

		return EXIT_FAILURE;
	}

	struct hostent *host = gethostbyname(argv[1]);

	if (host == NULL) {
		fprintf(stderr, "[ERROR #2] Invalid address (Server IP)\n");

		return EXIT_FAILURE;
	}

	int server_port = atoi(argv[2]);

	if (server_port <= 0) {
		fprintf(stderr, "[ERROR #3] Invalid host port number\n");

		return EXIT_FAILURE;
	}

	int udp_socket_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if (udp_socket_fd == -1) {
		fprintf(stderr, "[ERROR #4] Failed to create socket: %s\n", strerror(errno));

		return EXIT_FAILURE;
	}

	socklen_t sockaddr_size = sizeof(struct sockaddr);
	struct sockaddr_in server_address;

	bzero(&server_address, sockaddr_size);

	server_address.sin_family = AF_INET;
	server_address.sin_addr = *((struct in_addr *)host->h_addr_list[0]);
	server_address.sin_port = htons(server_port);

	if (bind(udp_socket_fd, (struct sockaddr *) &server_address, sockaddr_size) == -1) {
		fprintf(stderr, "[ERROR #5] Failed to bind socket: %s\n", strerror(errno));

		return EXIT_FAILURE;
	}

	char input_buffer[BUFFER_SIZE] = {0};
	char output_buffer[BUFFER_SIZE] = {0};

	do {
		fprintf(stdout, "[INFO #1] Server waiting message...\n", input_buffer);
		// Receive a message from client
		int read_bytes = recvfrom(udp_socket_fd, input_buffer, BUFFER_SIZE, 
							0, (struct sockaddr *)(&server_address), &sockaddr_size);
		if (read_bytes < 0) {
			fprintf(stderr, "[ERROR #6] Failed receiving message: %s\n", strerror(errno));
			
			break;
		}

		reverse_string(input_buffer, output_buffer);

		if (sendto(udp_socket_fd, output_buffer, strlen(output_buffer) + 1, 
				0, (struct sockaddr *)(&server_address), sockaddr_size) == -1) {
			fprintf(stderr, "[ERROR #7] Failed sending message: %s\n", strerror(errno));
			break;
		}

		// Stop sending message to server
		if (strcmp("STOP", input_buffer) == 0) {
			break;
		}

		fprintf(stdout, "[INFO #2] Received a message from server: %s\n", input_buffer);
	} while (1);


	if (close(udp_socket_fd) == -1) {
		fprintf(stderr, "[ERROR #8] Failed to close socket: %s\n", strerror(errno));

		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

void reverse_string(char *input, char *output) {

	int len_input_str = strlen(input);

	for (int i = 0; i < len_input_str; i++) {
		output[i] = input[len_input_str - i - 1];
	}

	output[len_input_str] = '\0';

	return;
}