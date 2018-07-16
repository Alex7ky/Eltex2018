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
#include <poll.h>

#define TRUE                    1
#define FALSE                   0

#define MAX_QUEUE_CONNECTIONS   4
#define MAX_CONNECTIONS         64
#define BUFFER_SIZE             1024

int accept_connections(int tcp_socket_fd, int udp_socket_fd);
void check_tcp_connections(struct pollfd *pfd, int *maxi);
void check_tcp_msg(struct pollfd *pfd, int maxi);
void check_udp_msg(struct pollfd *pfd);
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

int accept_connections(int tcp_socket_fd, int udp_socket_fd) {

    struct pollfd pfd[MAX_CONNECTIONS];
    int maxi = 1;  
    int nready = 0;
    int i;

    pfd[0].fd = tcp_socket_fd;
    pfd[0].events = POLLIN;

    pfd[1].fd = udp_socket_fd;
    pfd[1].events = POLLIN;
    
    for (int i = 2; i < MAX_CONNECTIONS; i++)
        pfd[i].fd = -1;
    
    while (TRUE) {
        nready = poll(pfd, maxi + 1, NULL);

        if (nready == -1) {
            fprintf(stderr, "[ERROR #11] Failed to poll: %s\n", strerror(errno));

            return EXIT_FAILURE;
        }

        check_tcp_connections(pfd, &maxi);

        check_tcp_msg(pfd, maxi);

        check_udp_msg(pfd);
     
    }

    return EXIT_SUCCESS;
}

void check_tcp_connections(struct pollfd *pfd, int *maxi)
{
    if (pfd[0].revents & POLLIN) {   /* new client connection */
        struct sockaddr_in client_address;
        socklen_t sockaddr_size = sizeof(struct sockaddr);
        int client_socket_fd = accept(pfd[0].fd, (struct sockaddr *)(&client_address), &sockaddr_size);

        if (client_socket_fd == -1) {
            fprintf(stderr, "[ERROR #12][TCP] Failed to accept a socket from client: %s\n", strerror(errno));

            return;
        }

        int i;
        
        for (i = 2; i < MAX_CONNECTIONS; i++) {
            if (pfd[i].fd < 0) {
                pfd[i].fd = client_socket_fd;
                pfd[i].events = POLLIN;
                
                if (i > *maxi)
                    *maxi = i;

                break;
            }
        }

        if (i == MAX_CONNECTIONS) {
            fprintf(stderr, "[ERROR #13][TCP] N_CLIENTS == MAX_CONNECTIONS");

            return;
        }

        fprintf(stdout, "[INFO #2][TCP] Connection established with %s:%d\n", 
            inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));
    }

    return;
}

void check_tcp_msg(struct pollfd *pfd, int maxi) {
    struct sockaddr_in client_address;
    socklen_t sockaddr_size = sizeof(struct sockaddr);

    char input_buffer[BUFFER_SIZE] = {0};
    char output_buffer[BUFFER_SIZE] = {0};

    for (int i = 2; i <= maxi; i++) {   /* check all clients */
        if (pfd[i].fd < 0)
            continue;

        if (pfd[i].revents & POLLIN) {
            int read_bytes = recv(pfd[i].fd, input_buffer, BUFFER_SIZE, 0);
                
            if (read_bytes < 0) {
                fprintf(stderr, "[ERROR #14][TCP] Failed sending message to the client %s:%d: %s\n", 
                inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port), strerror(errno));
                    continue;
            }

            fprintf(stdout, "[INFO #3][TCP] Received a message from client %s:%d: %s\n", 
                inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port), input_buffer);

            if (read_bytes == 0 || strcmp("BYE", input_buffer) == 0) {
                if (close(pfd[i].fd) == -1) {
                    fprintf(stderr, "[ERROR #15] Failed to close socket: %s\n", strerror(errno));
                } else {
                    fprintf(stdout, "[INFO #4][TCP] Client %s:%d disconnected.\n", 
                    inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));
                }                    
            } else {
                reverse_string(input_buffer, output_buffer);
                if (send(pfd[i].fd, output_buffer, strlen(output_buffer), 0) == -1 ) {
                    fprintf(stderr, "[ERROR #16] Failed sending message to the client %s:%d: %s\n", 
                    inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port), strerror(errno));
                }
            }
        }
    }

    return;
}

void check_udp_msg(struct pollfd *pfd) {
    if (pfd[1].revents & POLLIN) {
        struct sockaddr_in client_address;
        socklen_t sockaddr_size = sizeof(struct sockaddr);

        char input_buffer[BUFFER_SIZE] = {0};
        char output_buffer[BUFFER_SIZE] = {0};

        int read_bytes = recvfrom(pfd[1].fd, input_buffer, BUFFER_SIZE, 0, (struct sockaddr *)(&client_address), &sockaddr_size);

        if (read_bytes < 0) {
            fprintf(stderr, "[ERROR #17][UDP] Failed sending message to the client %s:%d: %s\n", 
                inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port), strerror(errno));
            
            return;
        }

        fprintf(stdout, "[INFO #5][UDP] Received a message from client %s:%d: %s\n", 
            inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port), input_buffer);

        reverse_string(input_buffer, output_buffer);

        if (sendto(pfd[1].fd, output_buffer, strlen(output_buffer), 0, (struct sockaddr *)(&client_address), sockaddr_size) == -1) {
            fprintf(stderr, "[ERROR #18][UDP] Failed sending message to the client %s:%d: %s\n", 
                inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port), strerror(errno));
        }
    }
}

void reverse_string(char *input, char *output) {

	int len_input_str = strlen(input);

    for (int i = 0; i < len_input_str; i++) {
        output[i] = input[len_input_str - i - 1];
    }

    output[len_input_str] = '\0';

    return;
}