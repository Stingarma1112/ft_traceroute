#include "../include/ft_traceroute.h"

int send_packet_with_ttl(t_traceroute *traceroute, int ttl) {
	int packet_size;
	char *packet;
	struct icmphdr *icmp;
	ssize_t sent;

	if (setsockopt(traceroute->socket_fd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl)) < 0) {
		perror("ft_traceroute: setsockopt: IP_TTL");
		return 1;
	}

	packet_size = 8 + 56;
	packet = (char *)malloc(packet_size);
	if (!packet) {
		perror("ft_traceroute: error allocating memory for packet");
		return 1;
	}

	ft_memset(packet, 0, packet_size);
	icmp = (struct icmphdr *)packet;

	build_icmp_packet(icmp, traceroute->sequence, traceroute->pid, packet_size);

	sent = sendto(traceroute->socket_fd, packet, packet_size, 0, (struct sockaddr *)&traceroute->target_addr, sizeof(traceroute->target_addr));

	free(packet);
	
	if (sent < 0) {
		perror("ft_traceroute: sendto");
		return 1;
	}
	if (sent != packet_size) {
		fprintf(stderr, "ft_traceroute: sent %zd bytes, expected %d\n", sent, packet_size);
		return 1;
	}
	traceroute->stats.packets_send++;
	traceroute->sequence++;
	return 0;
}
