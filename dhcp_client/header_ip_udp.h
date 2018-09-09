#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <string.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <net/if.h>

#define ETH_FRAME_SIZE 342 
#define SIZE_IP_UDP_PACKET 328 //ip_hdr 20 + udp_hdr 8 + bootp_hdr 292

#define DS_MAC0	0xff
#define DS_MAC1	0xff
#define DS_MAC2	0xff
#define DS_MAC3	0xff
#define DS_MAC4	0xff
#define DS_MAC5	0xff

#define SR_MAC0	0x04
#define SR_MAC1	0xb1
#define SR_MAC2	0x67
#define SR_MAC3	0x1a
#define SR_MAC4	0xc9
#define SR_MAC5	0xa0

#define SOUR_PORT 68
#define DEST_PORT 67

#define SOUR_ADDR "0.0.0.0"
#define DEST_ADDR "255.255.255.255"


typedef struct ip_header {
#if __BYTE_ORDER == __LITTLE_ENDIAN
	unsigned int ihl:4;
	unsigned int version:4;
#elif __BYTE_ORDER == __BIG_ENDIAN
	unsigned int version:4;
	unsigned int ihl:4;
#else
# error	"Please fix <bits/endian.h>"
#endif
	uint8_t tos;
	uint16_t tot_len;
	uint16_t id;
	uint16_t frag_off;
	uint8_t ttl;
	uint8_t protocol;
	uint16_t check;
	uint32_t saddr;
	uint32_t daddr;
} IP_HDR;
 
typedef struct udp_header {
	uint16_t source_port;
	uint16_t dest_port;
	uint16_t udp_len;
	uint16_t udp_sum;
} UDP_HDR;

typedef struct bootp_pkt {		/* BOOTP packet format */
	uint8_t op;			    /* 1 = request, 2 = reply */
	uint8_t htype;		    /* HW address type */
	uint8_t hlen;		    /* HW address length */
	uint8_t hops;		    /* Used only by gateways */
	uint32_t xid;		    /* Transaction ID */
	uint16_t secs;		    /* Seconds since we started */
	uint16_t flags;		    /* Just what it says */
	uint32_t client_ip;		/* Client's IP address if known */
	uint32_t your_ip;		/* Assigned IP address */
	uint32_t server_ip;		/* (Next, e.g. NFS) Server's IP address */
	uint32_t relay_ip;		/* IP address of BOOTP relay */
	uint8_t hw_addr[16];    /* Client's HW address */
	uint8_t serv_name[64];	/* Server host name */
	uint8_t boot_file[128];	/* Name of boot file */
	uint8_t exten[64];		/* DHCP options / BOOTP vendor extensions */
} BOOTP_HDR;

uint16_t checksum(uint16_t *, int);
uint16_t create_packet_eth_ip_udp(char *);
