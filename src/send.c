#include "../include/ft_traceroute.h"

int send_packet(t_ping *ping) {
	int packet_size;
	char *packet;
	struct icmphdr *icmp;
	ssize_t sent;

	packet_size = 8 + ping->options.packet_size;
	packet = (char *)malloc(packet_size);
	if (!packet) {
		perror("ft_ping: error allocating memory for packet");
		return 1;
	}

	memset(packet, 0, packet_size);
	icmp = (struct icmphdr *)packet;

	build_icmp_packet(icmp, ping->sequence, ping->pid, packet_size);

	sent = sendto(ping->socket_fd, packet, packet_size, 0, (struct sockaddr *)&ping->target_addr, sizeof(ping->target_addr));

	free(packet);
	
	if (sent < 0) {
		perror("ft_ping: sendto");
		return 1;
	}
	if (sent != packet_size) {
		fprintf(stderr, "ft_ping: sent %zd bytes, expected %d\n", sent, packet_size);
		return 1;
	}
	ping->stats.packets_send++;
	return 0;
}
