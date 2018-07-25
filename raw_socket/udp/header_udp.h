#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <string.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define SIZE_UDP_PACKET 4076
#define SIZE_MSG 4068
#define CLIENT_PORT 6666
#define SERVER_PORT 8622

#define D_ADDR "127.0.0.1"
 
typedef struct udp_header {
	uint16_t source_port;
	uint16_t dest_port;
	uint16_t udp_len;
	uint16_t udp_sum;
} UDP_HDR;

uint16_t checksum(uint16_t *, int);
uint16_t create_packet_udp(char *);
