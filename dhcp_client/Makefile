CC=gcc
CFLAGS=-g -c -Wall -pedantic

all: main_raw

main_raw: main_client_eth_ip_udp.o create_packet_eth_ip_udp.o checksum.o
	$(CC) main_client_eth_ip_udp.o create_packet_eth_ip_udp.o checksum.o -o main_raw

main_client_eth_ip_udp.o: main_client_eth_ip_udp.c
	$(CC) $(CFLAGS) main_client_eth_ip_udp.c

create_packet_eth_ip_udp.o: create_packet_eth_ip_udp.c
	$(CC) $(CFLAGS) create_packet_eth_ip_udp.c

checksum.o: checksum.c
	$(CC) $(CFLAGS) checksum.c

clean:
	rm -rf *.o main_raw
