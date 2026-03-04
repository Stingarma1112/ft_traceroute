#include "../include/ft_traceroute.h"

int send_packet_with_ttl(t_traceroute *traceroute, int ttl) {
	unsigned char payload[32];
	struct sockaddr_in dest_addr;
	ssize_t sent;
	uint16_t dest_port;

	if (setsockopt(traceroute->send_socket_fd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl)) < 0) {
		perror("ft_traceroute: setsockopt: IP_TTL");
		return 1;
	}
	dest_addr = traceroute->target_addr;
	dest_port = (uint16_t)(TR_UDP_BASE_PORT + traceroute->sequence);
	dest_addr.sin_port = htons(dest_port);
	ft_bzero(payload, sizeof(payload));
	sent = sendto(traceroute->send_socket_fd, payload, sizeof(payload), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
	
	if (sent < 0) {
		perror("ft_traceroute: sendto");
		return 1;
	}
	if (sent != (ssize_t)sizeof(payload)) {
		fprintf(stderr, "ft_traceroute: sent %zd bytes, expected %zu\n", sent, sizeof(payload));
		return 1;
	}
	traceroute->stats.packets_send++;
	traceroute->sequence++;
	return 0;
}
