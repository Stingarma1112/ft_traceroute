#include "../include/ft_ping.h"
#include <math.h>

void display_packet_received(t_ping *ping, double rtt, int ttl) {

	if (ping->options.quiet)
		return;

	char ip_str[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &ping->target_addr.sin_addr, ip_str, INET_ADDRSTRLEN);
	printf("%d bytes from %s: icmp_seq=%d ttl=%d time=%.3f ms\n", ping->options.packet_size, ip_str, ping->sequence, ttl, rtt);
}

void display_stats(t_ping *ping) {
	char ip_str[INET_ADDRSTRLEN];
	double packet_loss;
	double average_rtt;
	double stddev = 0.0;

	inet_ntop(AF_INET, &ping->target_addr.sin_addr, ip_str, INET_ADDRSTRLEN);

	printf("--- %s ping statistics ---\n", ping->target_host);
	printf("%d packets transmitted, %d packets received", ping->stats.packets_send, ping->stats.packets_received);

	if (ping->stats.errors > 0) {
		printf(", %d errors", ping->stats.errors);
	}

	if (ping->stats.packets_send > 0) {
		packet_loss = ((double)(ping->stats.packets_send - ping->stats.packets_received) / ping->stats.packets_send) * 100;
		printf(", %.0f%% packet loss", packet_loss);
	}

	printf("\n");

	if (ping->stats.packets_received > 0) {
		average_rtt = ping->stats.total_rtt / ping->stats.packets_received;

		if (ping->stats.packets_received > 1) {
			double mean_squared = (ping->stats.total_rtt_squared / ping->stats.packets_received);
			double variance = mean_squared - (average_rtt * average_rtt);
			if (variance > 0) {
				stddev = sqrt(variance);
			}
		}

		printf("round-trip min/avg/max/stddev = %.3f/%.3f/%.3f/%.3f ms\n",
		       ping->stats.min_rtt, average_rtt, ping->stats.max_rtt, stddev);
	}
}
