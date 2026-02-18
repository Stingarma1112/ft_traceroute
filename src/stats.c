#include "../include/ft_traceroute.h"

void init_stats(t_stats *stats) {
	stats->packets_send = 0;
	stats->packets_received = 0;
	stats->min_rtt = -1;
	stats->max_rtt = 0;
	stats->total_rtt = 0;
	stats->errors = 0;
}

void update_rtt_stats(t_stats *stats, double rtt) {
	if (stats->min_rtt < 0 || rtt < stats->min_rtt)
		stats->min_rtt = rtt;
	if (rtt > stats->max_rtt)
		stats->max_rtt = rtt;
	stats->total_rtt += rtt;
}