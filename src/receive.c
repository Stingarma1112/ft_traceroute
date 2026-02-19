#include "../include/ft_traceroute.h"

#define PACKET_SIZE_MAX 65535
#define IP_HEADER_SIZE 20

int receive_packet_for_hop(t_traceroute *traceroute, int ttl, struct timeval *send_time, double *rtt, struct sockaddr_in *router_addr) {

	(void)ttl;
	char buffer[PACKET_SIZE_MAX];
	struct iphdr *ip_header;
	struct icmphdr *icmp_reply;
	struct sockaddr_in from_addr;
	socklen_t from_len = sizeof(from_addr);
	struct timeval recv_time;
	ssize_t received;

	received = recvfrom(traceroute->socket_fd, buffer, sizeof(buffer), 0, (struct sockaddr *)&from_addr, &from_len);
	if (received < 0) {
		if (errno == EAGAIN || errno == EWOULDBLOCK) {
			return -1;
		}
		perror("ft_traceroute: recvfrom");
		return -1;
	}

	gettimeofday(&recv_time, NULL);

	*rtt = (recv_time.tv_sec - send_time->tv_sec) * 1000.0 + (recv_time.tv_usec - send_time->tv_usec) / 1000.0;
	if (received < (ssize_t)sizeof(struct iphdr))
		return -1;
	ip_header = (struct iphdr *)buffer;

	if (ip_header->version != 4) {
		return -1;
	}
	int ip_header_len = ip_header->ihl * 4;
	if (received < (ssize_t)(ip_header_len + 8))
		return -1;
	icmp_reply = (struct icmphdr *)(buffer + ip_header_len);

	*router_addr = from_addr;
	if (icmp_reply->type == ICMP_TIME_EXCEEDED)
		return 0;
	if (icmp_reply->type == ICMP_ECHOREPLY) {
		if (ntohs(icmp_reply->un.echo.id) != (uint16_t)traceroute->pid)
			return -1;
		return 1;
	}
	if (icmp_reply->type == ICMP_DEST_UNREACH)
		return 2;
	return -1;
}