#include "../include/ft_traceroute.h"

int create_socket(t_traceroute *traceroute) {
	int recv_sockfd;
	int send_sockfd;

	recv_sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (recv_sockfd < 0) {
		if (errno == EPERM) {
			fprintf(stderr, "ft_traceroute: socket: Operation not permitted\n");
			fprintf(stderr, "ft_traceroute: You may need to run as root\n");
		} else {
			perror("ft_traceroute: socket");
		}
		return 1;
	}
	send_sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (send_sockfd < 0) {
		perror("ft_traceroute: socket");
		close(recv_sockfd);
		return 1;
	}
	traceroute->socket_fd = recv_sockfd;
	traceroute->send_socket_fd = send_sockfd;
	return 0;
}