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
#include <sys/epoll.h>

#define TRUE                    1
#define FALSE                   0

#define MAX_QUEUE_CONNECTIONS   4
#define MAX_CONNECTIONS         64
#define BUFFER_SIZE             1024

int accept_connections(int tcp_socket_fd, int udp_socket_fd);
void check_tcp_connections(int epfd, struct epoll_event *evlist, int *max_index, int tcp_socket_fd);
void check_tcp_msg(int client_fd);
void check_udp_msg(int udp_socket_fd);
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

    struct epoll_event evlist[MAX_CONNECTIONS];
    struct epoll_event ev_tcp, ev_udp;
    struct epoll_event evlist_all[MAX_CONNECTIONS];
    int epfd;
    int nready = 0;
    int max_index = 1;
    int i;

    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        evlist[i].data.fd = -1;
        evlist_all[i].data.fd = -1;
    }

    evlist_all[0].events = EPOLLIN;
    evlist_all[0].data.fd = tcp_socket_fd;

    evlist_all[1].events = EPOLLIN;
    evlist_all[1].data.fd = udp_socket_fd;

    epfd = epoll_create(MAX_CONNECTIONS);

    if (epoll_ctl(epfd, EPOLL_CTL_ADD, tcp_socket_fd, &evlist_all[0]) == -1) {
        fprintf(stderr, "[ERROR #11] Failed to epoll_ctl tcp_socket_fd: %s\n", strerror(errno));

        return EXIT_FAILURE;
    }

    if (epoll_ctl(epfd, EPOLL_CTL_ADD, udp_socket_fd, &evlist_all[1]) == -1) {
        fprintf(stderr, "[ERROR #12] Failed to epoll_ctl udp_socket_fd: %s\n", strerror(errno));

        return EXIT_FAILURE;
    }

    if (epfd == -1 ) {
        fprintf(stderr, "[ERROR #13] Failed to epoll_create: %s\n", strerror(errno));

        return EXIT_FAILURE;
    }


    while (TRUE) {
        nready = epoll_wait(epfd, evlist, MAX_CONNECTIONS, NULL);

        if (nready == -1)
            fprintf(stderr, "[ERROR #14] Failed to poll: %s\n", strerror(errno));

        for (int i = 0; i <= max_index; i++) {
            if (evlist[i].events & EPOLLIN) {
                if (evlist[i].data.fd == tcp_socket_fd)
                    check_tcp_connections(epfd, evlist_all, &max_index, tcp_socket_fd);

                if (evlist[i].data.fd == evlist_all[i].data.fd && i >= 2)
                    check_tcp_msg(evlist_all[i].data.fd);

                if (evlist[i].data.fd == udp_socket_fd)
                    check_udp_msg(udp_socket_fd);
            }
        }  
        printf("TEST4\n"); 
    }

    return EXIT_SUCCESS;
}

void check_tcp_connections(int epfd, struct epoll_event *evlist, int *max_index, int tcp_socket_fd) {
    struct sockaddr_in client_address;
    socklen_t sockaddr_size = sizeof(struct sockaddr);
    int client_socket_fd = accept(tcp_socket_fd, (struct sockaddr *)(&client_address), &sockaddr_size);

    if (client_socket_fd == -1) {
        fprintf(stderr, "[ERROR #15][TCP] Failed to accept a socket from client: %s\n", strerror(errno));

        return;
    }

    int i;

    for (i = 2; i < MAX_CONNECTIONS; i++) {
        if (evlist[i].data.fd < 0) {
            evlist[i].data.fd = client_socket_fd;
            evlist[i].events = EPOLLIN | EPOLLET;

            if (epoll_ctl(epfd, EPOLL_CTL_ADD, client_socket_fd, &evlist[i]) == -1) {
                fprintf(stderr, "[ERROR #16] Failed to epoll_ctl tcp_socket_fd: %s\n", strerror(errno));

                return;
            }

            if (i > *max_index)
                *max_index = i;

            break;
        }
    }

    if (i == MAX_CONNECTIONS) {
        fprintf(stderr, "[ERROR #17][TCP] N_CLIENTS == MAX_CONNECTIONS");

        return;
    }

    fprintf(stdout, "[INFO #2][TCP] Connection established with %s:%d\n", 
        inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));

    return;
}

void check_tcp_msg(int client_fd) {
    struct sockaddr_in client_address;
    socklen_t sockaddr_size = sizeof(struct sockaddr);

    char input_buffer[BUFFER_SIZE] = {0};
    char output_buffer[BUFFER_SIZE] = {0};

    int read_bytes = recv(client_fd, input_buffer, BUFFER_SIZE, 0);
          
    if (read_bytes < 0) {
        fprintf(stderr, "[ERROR #18][TCP] Failed sending message to the client %s:%d: %s\n", 
        inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port), strerror(errno));
    }

    fprintf(stdout, "[INFO #3][TCP] Received a message from client %s:%d: %s\n", 
            inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port), input_buffer);

    if (read_bytes == 0 || strcmp("BYE", input_buffer) == 0) {
        if (close(client_fd) == -1) {
            fprintf(stderr, "[ERROR #19] Failed to close socket: %s\n", strerror(errno));
        } else {
            fprintf(stdout, "[INFO #4][TCP] Client %s:%d disconnected.\n", 
            inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));
        }                    
    } else {
        reverse_string(input_buffer, output_buffer);
        if (send(client_fd, output_buffer, strlen(output_buffer), 0) == -1 ) {
            fprintf(stderr, "[ERROR #20] Failed sending message to the client %s:%d: %s\n", 
            inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port), strerror(errno));
        }
    }

    return;
}

void check_udp_msg(int udp_socket_fd) {
    struct sockaddr_in client_address;
    socklen_t sockaddr_size = sizeof(struct sockaddr);

    char input_buffer[BUFFER_SIZE] = {0};
    char output_buffer[BUFFER_SIZE] = {0};

    int read_bytes = recvfrom(udp_socket_fd, input_buffer, BUFFER_SIZE, 0, (struct sockaddr *)(&client_address), &sockaddr_size);

    if (read_bytes < 0) {
        fprintf(stderr, "[ERROR #21][UDP] Failed sending message to the client %s:%d: %s\n", 
            inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port), strerror(errno));
        
        return;
    }

    fprintf(stdout, "[INFO #5][UDP] Received a message from client %s:%d: %s\n", 
        inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port), input_buffer);

    reverse_string(input_buffer, output_buffer);

    if (sendto(udp_socket_fd, output_buffer, strlen(output_buffer), 0, (struct sockaddr *)(&client_address), sockaddr_size) == -1) {
        fprintf(stderr, "[ERROR #22][UDP] Failed sending message to the client %s:%d: %s\n", 
            inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port), strerror(errno));
    }

    return;
}

void reverse_string(char *input, char *output) {

	int len_input_str = strlen(input);

    for (int i = 0; i < len_input_str; i++) {
        output[i] = input[len_input_str - i - 1];
    }

    output[len_input_str] = '\0';

    return;
}