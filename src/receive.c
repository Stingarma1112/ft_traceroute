#include "../include/ft_ping.h"

#define PACKET_SIZE_MAX 65535
#define IP_HEADER_SIZE 20

int receive_packet(t_ping *ping, struct timeval *send_time, double *rtt, int *ttl) {

	char buffer[PACKET_SIZE_MAX];
	struct iphdr *ip_header;
	struct icmphdr *icmp_reply;
	struct sockaddr_in from_addr;
	socklen_t from_len = sizeof(from_addr);
	struct timeval recv_time;
	ssize_t received;
	uint16_t receive_id;

	received = recvfrom(ping->socket_fd, buffer, sizeof(buffer), 0, (struct sockaddr *)&from_addr, &from_len);
	if (received < 0) {
		if (errno == EAGAIN || errno == EWOULDBLOCK) {
			return -1;
		}
		perror("ft_ping: recvfrom");
		return 1;
	}

	gettimeofday(&recv_time, NULL);

	*rtt = (recv_time.tv_sec - send_time->tv_sec) * 1000.0 + (recv_time.tv_usec - send_time->tv_usec) / 1000.0;
	ip_header = (struct iphdr *)buffer;

	if (ip_header->version != 4) {
		return 1;
	}
	int ip_header_len = ip_header->ihl * 4;
	icmp_reply = (struct icmphdr *)(buffer + ip_header_len);

	if (icmp_reply->type == ICMP_ECHOREPLY) {
		receive_id = ntohs(icmp_reply->un.echo.id);

		if (receive_id != ping->pid) {
			return 1;
		}
		ping->stats.packets_received++;

		if (ping->stats.min_rtt < 0 || *rtt < ping->stats.min_rtt)
			ping->stats.min_rtt = *rtt;
		if (*rtt > ping->stats.max_rtt)
			ping->stats.max_rtt = *rtt;
		ping->stats.total_rtt += *rtt;
		ping->stats.total_rtt_squared += (*rtt) * (*rtt);
		if (ttl != NULL)
			*ttl = ip_header->ttl;
		return 0;
	} else if (icmp_reply->type == ICMP_TIME_EXCEEDED || icmp_reply->type == ICMP_DEST_UNREACH) {
		ping->stats.errors++;

		if (ping->options.verbose) {
			char from_ip[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, &from_addr.sin_addr, from_ip, INET_ADDRSTRLEN);

			if (icmp_reply->type == ICMP_TIME_EXCEEDED)
				fprintf(stderr, "ft_ping: %s icmp_seq=%d Time to live exceeded\n", from_ip, ping->sequence);
			if (icmp_reply->type == ICMP_DEST_UNREACH)
				fprintf(stderr, "ft_ping: %s icmp_seq=%d Destination Unreachable\n", from_ip, ping->sequence);
		}
		return 2;
	}
	return 1;
}