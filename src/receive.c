#include "../include/ft_traceroute.h"

#define PACKET_SIZE_MAX 65535
#define IP_HEADER_SIZE 20

static int matches_original_probe(t_traceroute *traceroute, char *buffer, ssize_t received, int outer_ip_len, uint16_t expected_seq) {
	struct iphdr	*original_ip;
	struct udphdr	*original_udp;
	int				original_ip_len;
	int				offset;

	offset = outer_ip_len + (int)sizeof(struct icmphdr);
	if (received < (ssize_t)(offset + (int)sizeof(struct iphdr) + (int)sizeof(struct udphdr)))
		return 0;
	original_ip = (struct iphdr *)(buffer + offset);
	original_ip_len = original_ip->ihl * 4;
	if (original_ip_len < IP_HEADER_SIZE)
		return 0;
	if (original_ip->protocol != IPPROTO_UDP)
		return 0;
	if (original_ip->daddr != traceroute->target_addr.sin_addr.s_addr)
		return 0;
	offset += original_ip_len;
	if (received < (ssize_t)(offset + (int)sizeof(struct udphdr)))
		return 0;
	original_udp = (struct udphdr *)(buffer + offset);
	if (ntohs(original_udp->dest) != (uint16_t)(traceroute->options.base_port + expected_seq))
		return 0;
	return 1;
}

int receive_packet_for_hop(t_traceroute *traceroute, int ttl, struct timeval *send_time, struct timeval *hop_start_time, double *rtt, struct sockaddr_in *router_addr, uint16_t expected_seq) {

	(void)ttl;
	char			buffer[PACKET_SIZE_MAX];
	struct iphdr	*ip_header;
	struct icmphdr	*icmp_reply;
	struct sockaddr_in	from_addr;
	socklen_t		from_len;
	struct timeval		now;
	struct timeval		recv_time;
	struct timeval		timeout;
	fd_set			readfds;
	ssize_t			received;
	double			elapsed_ms;
	double			remaining_ms;
	int				ip_header_len;
	int				ret;

	while (1) {
		gettimeofday(&now, NULL);
		elapsed_ms = (now.tv_sec - hop_start_time->tv_sec) * 1000.0 + (now.tv_usec - hop_start_time->tv_usec) / 1000.0;
		remaining_ms = (double)traceroute->options.timeout_ms - elapsed_ms;
		if (remaining_ms <= 0.0)
			return -1;
		timeout.tv_sec = (int)(remaining_ms / 1000.0);
		timeout.tv_usec = (int)((remaining_ms - timeout.tv_sec * 1000.0) * 1000.0);
		FD_ZERO(&readfds);
		FD_SET(traceroute->socket_fd, &readfds);
		ret = select(traceroute->socket_fd + 1, &readfds, NULL, NULL, &timeout);
		if (ret < 0) {
			if (errno == EINTR)
				continue;
			perror("ft_traceroute: select");
			return -1;
		}
		if (ret == 0)
			return -1;
		from_len = sizeof(from_addr);
		received = recvfrom(traceroute->socket_fd, buffer, sizeof(buffer), 0, (struct sockaddr *)&from_addr, &from_len);
		if (received < 0) {
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				return -1;
			perror("ft_traceroute: recvfrom");
			return -1;
		}
		if (received < (ssize_t)sizeof(struct iphdr))
			continue;
		ip_header = (struct iphdr *)buffer;
		if (ip_header->version != 4)
			continue;
		ip_header_len = ip_header->ihl * 4;
		if (ip_header_len < IP_HEADER_SIZE)
			continue;
		if (received < (ssize_t)(ip_header_len + (int)sizeof(struct icmphdr)))
			continue;
		icmp_reply = (struct icmphdr *)(buffer + ip_header_len);
		if (icmp_reply->type == ICMP_TIME_EXCEEDED) {
			if (!matches_original_probe(traceroute, buffer, received, ip_header_len, expected_seq))
				continue;
			*router_addr = from_addr;
			gettimeofday(&recv_time, NULL);
			*rtt = (recv_time.tv_sec - send_time->tv_sec) * 1000.0 + (recv_time.tv_usec - send_time->tv_usec) / 1000.0;
			return 0;
		}
		if (icmp_reply->type == ICMP_DEST_UNREACH) {
			if (!matches_original_probe(traceroute, buffer, received, ip_header_len, expected_seq))
				continue;
			*router_addr = from_addr;
			gettimeofday(&recv_time, NULL);
			*rtt = (recv_time.tv_sec - send_time->tv_sec) * 1000.0 + (recv_time.tv_usec - send_time->tv_usec) / 1000.0;
			if (icmp_reply->code == ICMP_PORT_UNREACH)
				return 2;
			return 2;
		}
	}
}