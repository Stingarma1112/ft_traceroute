#include "../include/ft_traceroute.h"
#define PACKET_SIZE_DISPLAY 60

void display_header(t_traceroute *traceroute) {
	char ip_str[INET_ADDRSTRLEN];

	inet_ntop(AF_INET, &traceroute->target_addr.sin_addr, ip_str, INET_ADDRSTRLEN);
	printf("ft_traceroute to %s (%s), %d hops max, %d byte packets\n", traceroute->target_host, ip_str, traceroute->options.max_hops, PACKET_SIZE_DISPLAY);
}

void display_hop(t_traceroute *traceroute, int ttl) {
	t_hop *hop;
	char ip_str[INET_ADDRSTRELEN];
	int i;

	hop = &traceroute->hops[ttl - 1];
	printf("%2d ", ttl);

	for (i = 0; i < traceroute->options.probes_per_hop; i++) {
		if (hop->rtt[i] >= 0) {
			inet_ntop(AF_INET, &hop->router_addr[i].sin_addr, ip_str, INET_ADDRSTRLEN);
			if (hop->hostname[i])
				printf("%s (%s) ", hop->hostname[i], ip_str);
			else
				printf("%s (%s) ", ip_str, ip_str);
			printf("%.3f ms ", hop->rtt[i]);
		}
		else
			printf("* ");
	}
	printf("\n");
}

void display_stats(t_traceroute *traceroute) {
	(void)traceroute;
}
