#include "../include/ft_traceroute.h"
#define PACKET_SIZE_DISPLAY 60

void display_header(t_traceroute *traceroute) {
	char ip_str[INET_ADDRSTRLEN];

	inet_ntop(AF_INET, &traceroute->target_addr.sin_addr, ip_str, INET_ADDRSTRLEN);
	printf("ft_traceroute to %s (%s), %d hops max, %d byte packets\n", traceroute->target_host, ip_str, traceroute->options.max_hops, PACKET_SIZE_DISPLAY);
}

static int compare_addrs(struct sockaddr_in *addr1, struct sockaddr_in *addr2) {
	return (addr1->sin_addr.s_addr == addr2->sin_addr.s_addr);
}

void display_hop(t_traceroute *traceroute, int ttl) {
	t_hop *hop;
	char ip_str[INET_ADDRSTRLEN];
	char *hostname;
	int i;
	int show_addr;
	int last_reply_valid = 0;
	struct sockaddr_in last_reply_addr;

	hop = &traceroute->hops[ttl - 1];
	printf("%2d  ", ttl);

	for (i = 0; i < traceroute->options.probes_per_hop; i++) {
		if (hop->rtt[i] < 0) {
			printf(" *");
			continue;
		}
		show_addr = 0;
		if (i == 0 || hop->rtt[i - 1] < 0 || !last_reply_valid) {
			show_addr = 1;
		} else if (!compare_addrs(&hop->router_addr[i], &last_reply_addr)) {
			show_addr = 1;
		}
		if (show_addr) {
			inet_ntop(AF_INET, &hop->router_addr[i].sin_addr, ip_str, INET_ADDRSTRLEN);
			hostname = hop->hostname[i];
			if (hostname) {
				if (i == 0)
					printf("%s (%s)", hostname, ip_str);
				else
					printf(" %s (%s)", hostname, ip_str);
			} else {
				if (i == 0)
					printf("%s (%s)", ip_str, ip_str);
				else
					printf(" %s (%s)", ip_str, ip_str);
			}
		}
		printf("  %.3f ms", hop->rtt[i]);
		last_reply_addr = hop->router_addr[i];
		last_reply_valid = 1;
	}
	printf("\n");
}

void display_stats(t_traceroute *traceroute) {
	(void)traceroute;
}
