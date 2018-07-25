#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>

#define TRUE                    1
#define FALSE                   0

#define MAX_QUEUE_CONNECTIONS   4
#define MAX_CONNECTIONS         64
#define BUFFER_SIZE             1024


int max(int a, int b);
int accept_connections(int tcp_socket_fd, int udp_socket_fd);
void check_tcp_connections(int tcp_socket_fd, fd_set read_fd_set, int *array_client_socket_fd);
void check_tcp_msg(int tcp_socket_fd, fd_set read_fd_set, int *array_client_socket_fd);
void check_udp_msg(int udp_socket_fd, fd_set read_fd_set);
int reg_new_socket(int client_socket_fd, int* array_client_socket_fd, int n);
void reverse_string(char* input, char* output);


int main(int argc, char *argv[]) 
{

	if (argc != 2) {
		fprintf(stderr, "[ERROR #1] Non-worm number of arguments\n");

		return EXIT_FAILURE;
	} 

	int server_port = atoi(argv[1]);

	if (server_port <= 0) {
		fprintf(stderr, "[ERROR #2] Invalid host port number\n");

		return EXIT_FAILURE;
	}

	int tcp_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	int udp_socket_fd = socket(AF_INET, SOCK_DGRAM, 0);

	if (tcp_socket_fd == -1) {
		fprintf(stderr, "[ERROR #3] Failed to create socket TCP: %s\n", strerror(errno));
		return EXIT_FAILURE;
	}

	if (udp_socket_fd == -1) {
		fprintf(stderr, "[ERROR #4] Failed to create socket UDP: %s\n", strerror(errno));
		return EXIT_FAILURE;
	}

	
	socklen_t sockaddr_size = sizeof(struct sockaddr);

	struct sockaddr_in server_address;

	bzero(&server_address, sockaddr_size);
	
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);
	server_address.sin_port = htons(server_port);

	fprintf(stdout, "[INFO #1] Server address: %s\n", inet_ntoa(server_address.sin_addr)); 

	int option_value = 1;
	
	/* Reuse the port after the program ends */
	setsockopt(tcp_socket_fd, SOL_SOCKET, SO_REUSEADDR, &option_value, sizeof(option_value));

	if (bind(tcp_socket_fd, (struct sockaddr*)(&server_address), sockaddr_size) == -1) {
		fprintf(stderr, "[ERROR #5] Failed to bind TCP socket: %s\n", strerror(errno));
 
		return EXIT_FAILURE;
	}

	if (bind(udp_socket_fd, (struct sockaddr*)(&server_address), sockaddr_size) == -1) {
		fprintf(stderr, "[ERROR #6] Failed to bind UDP socket: %s\n", strerror(errno));
 
		return EXIT_FAILURE;
	}

	if (listen(tcp_socket_fd, MAX_QUEUE_CONNECTIONS) == -1) {
		fprintf(stderr, "[ERROR #7] Failed to listen to the TCP socket: %s\n", strerror(errno));
		return EXIT_FAILURE;
	}

	int ret_code = accept_connections(tcp_socket_fd, udp_socket_fd);

	if (ret_code == -1) {
		fprintf(stderr, "[ERROR #8] Server exit with an error: %s\n", strerror(errno));
	}

	if (close(tcp_socket_fd) == -1)  {
		fprintf(stderr, "[ERROR #9] Failed to close socket TCP: %s\n", strerror(errno));

		return EXIT_FAILURE;
	}

	if (close(udp_socket_fd) == -1)  {
		fprintf(stderr, "[ERROR #10] Failed to close socket UDP: %s\n", strerror(errno));

		return EXIT_FAILURE;
	}
	

	return EXIT_SUCCESS;
}

int max(int a, int b) {
	return (a > b) ? a : b;
}

int accept_connections(int tcp_socket_fd, int udp_socket_fd) {

	fd_set read_fd_set;

	int array_client_socket_fd[MAX_CONNECTIONS] = {0};

	while (TRUE) {
		/* Clear read_fd_set.*/
		FD_ZERO(&read_fd_set);

		/* Add tcp_socket_fd and udp_socket_fd to the set. */
		FD_SET(tcp_socket_fd, &read_fd_set);
		FD_SET(udp_socket_fd, &read_fd_set);
 
		int max_fd = max(tcp_socket_fd, udp_socket_fd);

		for (int i = 0; i < MAX_CONNECTIONS; ++ i) {
			int client_socket_fd = array_client_socket_fd[i];

			if (client_socket_fd > 0) {
				FD_SET(client_socket_fd, &read_fd_set);
			}
			
			if (client_socket_fd > max_fd) {
				max_fd = client_socket_fd;
			}
		}


		int events = select(max_fd + 1, &read_fd_set, NULL, NULL, NULL);
		
		if (events == -1) {
			fprintf(stderr, "[ERROR #11] Failed to select: %s\n", strerror(errno));

			return EXIT_FAILURE;
		}

		check_tcp_connections(tcp_socket_fd, read_fd_set, array_client_socket_fd);

		check_tcp_msg(tcp_socket_fd, read_fd_set, array_client_socket_fd);
		
		check_udp_msg(udp_socket_fd, read_fd_set);
	}

	return EXIT_SUCCESS;
}

void check_tcp_connections(int tcp_socket_fd, fd_set read_fd_set, int *array_client_socket_fd)
{
	if (FD_ISSET(tcp_socket_fd, &read_fd_set)) {

		struct sockaddr_in client_address;
		socklen_t sockaddr_size = sizeof(struct sockaddr);

		int client_socket_fd = accept(tcp_socket_fd, (struct sockaddr *)(&client_address), &sockaddr_size);

		if (client_socket_fd == -1) {
			fprintf(stderr, "[ERROR #12][TCP] Failed to accept a socket from client: %s\n", strerror(errno));

			return;
		}

		fprintf(stdout, "[INFO #2][TCP] Connection established with %s:%d\n", 
			inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));

		/* Register fd for sockets (get the index of an empty array cell from the array_client_socket_fd) */
		int client_socket_fd_index = reg_new_socket(client_socket_fd, array_client_socket_fd, MAX_CONNECTIONS);

		if (client_socket_fd_index == -1) {
			fprintf(stderr, "[ERROR #13][TCP] Failed to register the socket for client: %s:%d\n", 
				inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));
		} else {
			fprintf(stdout, "[INFO #3][TCP] Socket #%d registered for the client: %s:%d\n", 
				client_socket_fd_index, inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));
		}
	}

	return;
}

void check_tcp_msg(int tcp_socket_fd, fd_set read_fd_set, int *array_client_socket_fd) {
	struct sockaddr_in client_address;

	char input_buffer[BUFFER_SIZE] = {0};
	char output_buffer[BUFFER_SIZE] = {0};

	for (int i = 0; i < MAX_CONNECTIONS; ++ i) {
		int client_socket_fd = array_client_socket_fd[i];

		if (FD_ISSET(client_socket_fd, &read_fd_set)) {

			int read_bytes = recv(client_socket_fd, input_buffer, BUFFER_SIZE, 0);
				
			if (read_bytes < 0) {
				fprintf(stderr, "[ERROR #14][TCP] Failed sending message to the client %s:%d: %s\n", 
				inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port), strerror(errno));
					continue;
			}
			fprintf(stdout, "[INFO #4][TCP] Received a message from client %s:%d: %s\n", 
				inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port), input_buffer);

			if (read_bytes == 0 || strcmp("BYE", input_buffer) == 0) {
				if (close(client_socket_fd) == -1) {
					fprintf(stderr, "[ERROR #15] Failed to close socket: %s\n", strerror(errno));
				} else {
					fprintf(stdout, "[INFO #5][TCP] Client %s:%d disconnected.\n", 
					inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));
				}

				array_client_socket_fd[i] = 0;

				continue;
			} else {
				reverse_string(input_buffer, output_buffer);

				if (send(client_socket_fd, output_buffer, strlen(output_buffer), 0) == -1 ) {
					array_client_socket_fd[i] = 0;

					fprintf(stderr, "[ERROR #16] Failed sending message to the client %s:%d: %s\n", 
					inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port), strerror(errno));
				}
			}
		}
	}

	return;
}

void check_udp_msg(int udp_socket_fd, fd_set read_fd_set) {

	if (FD_ISSET(udp_socket_fd, &read_fd_set)) {

		struct sockaddr_in client_address;
		socklen_t sockaddr_size = sizeof(struct sockaddr);

		char input_buffer[BUFFER_SIZE] = {0};
		char output_buffer[BUFFER_SIZE] = {0};

		int read_bytes = recvfrom(udp_socket_fd, input_buffer, BUFFER_SIZE, 0, (struct sockaddr *)(&client_address), &sockaddr_size);

		if (read_bytes < 0) {
			fprintf(stderr, "[ERROR #17][UDP] Failed sending message to the client %s:%d: %s\n", 
				inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port), strerror(errno));
			
			return;
		}

		fprintf(stdout, "[INFO #6][UDP] Received a message from client %s:%d: %s\n", 
			inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port), input_buffer);

		reverse_string(input_buffer, output_buffer);

		if (sendto(udp_socket_fd, output_buffer, strlen(output_buffer), 0, (struct sockaddr *)(&client_address), sockaddr_size) == -1) {
			fprintf(stderr, "[ERROR #18][UDP] Failed sending message to the client %s:%d: %s\n", 
				inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port), strerror(errno));
		}
	}

	return;
}

int reg_new_socket(int client_socket_fd, int *array_client_socket_fd, int n) {

	for (int i = 0; i < n; ++ i) {
		if (array_client_socket_fd[i] == 0) {
			array_client_socket_fd[i] = client_socket_fd;
			
			return i;
		}
	}

	return -1;
}

void reverse_string(char *input, char *output) {

	int len_input_str = strlen(input);

	for (int i = 0; i < len_input_str; i++) {
		output[i] = input[len_input_str - i - 1];
	}

	output[len_input_str] = '\0';

	return;
}