#include "../include/ft_traceroute.h"

int parse_arguments(int argc, char **argv, t_traceroute *traceroute) {
	int i;
	traceroute->options.help = 0;
	traceroute->options.max_hops = 30;
	traceroute->options.probes_per_hop = 3;
	traceroute->target_host = NULL;

	for (i = 1; i < argc; i++) {
		if (ft_strcmp(argv[i], "--help") == 0) {
			traceroute->options.help = 1;
		} else if (ft_strcmp(argv[i][0] ==  '-')){
			fprintf(stderr, "ft_traceroute: invalid option -- '%s'\n", argv[i]);
			fprintf(stderr, "Try 'ft_traceroute --help' for more information.\n");
			return 1;
		} else {
			if (traceroute->target_host != NULL) {
				fprintf(stderr, "ft_traceroute: only one target is allowed\n");
				return 1;
			}
			traceroute->target_host = argv[i];
		}
	}
	if (traceroute->options.help) {
		print_help(argv[0]);
		return 2;
	}
	if (traceroute->target_host == NULL) {
		fprintf(stderr, "ft_traceroute: missing operand\n");
		fprintf(stderr, "Try 'ft_traceroute --help' for more information.\n");
		return 1;
	}
	return 0;
}

void print_usage(char *prog_name) {
	printf("Usage: %s [OPTION...] HOST ...\n", prog_name);
	printf("Send ICMP TIME_EXCEEDED packets to network hosts\n");
	printf("  --help         give this help list\n\n");
}

void print_help(char *prog_name) {
	print_usage(prog_name);
}