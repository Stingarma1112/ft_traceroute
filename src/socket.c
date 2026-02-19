#include "../include/ft_traceroute.h"

int create_socket(t_traceroute *traceroute) {
	int sockfd;

	sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (sockfd < 0) {
		if (errno == EPERM) {
			fprintf(stderr, "ft_traceroute: socket: Operation not permitted\n");
			fprintf(stderr, "ft_traceroute: You may need to run as root\n");
		} else {
			perror("ft_traceroute: socket");
		}
		return 1;
	}
	traceroute->socket_fd = sockfd;
	return 0;
}