#include "../include/ft_ping.h"

int create_socket(t_ping *ping) {
	int sockfd;
	int ttl = 64; // Default TTL Linux

	sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (sockfd < 0) {
		if (errno == EPERM) {
			fprintf(stderr, "ft_ping: socket: Operation not permitted\n");
			fprintf(stderr, "ft_ping: You may need to run as root\n");
		} else {
			perror("ft_ping: socket");
		}
		return 1;
	}
	if (ping->options.ttl_value > 0)
		ttl = ping->options.ttl_value;
	if (setsockopt(sockfd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl)) < 0) {
		perror("ft_ping: setsockopt: IP_TTL");
		close(sockfd);
		return 1;
	}
	ping->socket_fd = sockfd;
	return 0;
}