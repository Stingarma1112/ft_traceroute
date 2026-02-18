#include "../include/ft_ping.h"

#define ICMP_HEADER_SIZE 8

void build_icmp_packet(struct icmphdr *icmp, uint16_t sequence, pid_t pid, int packet_size) {
	memset(icmp, 0, ICMP_HEADER_SIZE);
	icmp->type = ICMP_ECHO;
	icmp->code = 0;
	icmp->un.echo.id = htons(pid);
	icmp->un.echo.sequence = htons(sequence);
	icmp->checksum = 0;
	icmp->checksum = calculate_checksum(icmp, packet_size);
}