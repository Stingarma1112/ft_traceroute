#include "../include/ft_traceroute.h"

void init_stats(t_stats *stats) {
	stats->packets_send = 0;
	stats->packets_received = 0;
	stats->errors = 0;
	stats->current_hop = 0;
}

void update_rtt_stats(t_stats *stats, double rtt) {
	(void)stats;
	(void)rtt;
}