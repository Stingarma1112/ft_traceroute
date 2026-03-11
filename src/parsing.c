#include "../include/ft_traceroute.h"

static int	is_numeric_string(const char *s) {
	int	i;

	if (!s || s[0] == '\0')
		return (0);
	i = 0;
	while (s[i]) {
		if (s[i] < '0' || s[i] > '9')
			return (0);
		i++;
	}
	return (1);
}

static int	parse_positive_int(const char *s, int *value) {
	long long	acc;
	int			i;

	if (!is_numeric_string(s))
		return (0);
	acc = 0;
	i = 0;
	while (s[i]) {
		acc = acc * 10 + (s[i] - '0');
		if (acc > 2147483647LL)
			return (0);
		i++;
	}
	*value = (int)acc;
	return (1);
}

static int	parse_option_value(int argc, char **argv, int *index, int *value, const char *opt_name) {
	if (*index + 1 >= argc) {
		fprintf(stderr, "ft_traceroute: option requires an argument -- '%s'\n", opt_name);
		return (0);
	}
	if (!parse_positive_int(argv[*index + 1], value)) {
		fprintf(stderr, "ft_traceroute: invalid numeric value '%s' for option '%s'\n", argv[*index + 1], opt_name);
		return (0);
	}
	*index += 1;
	return (1);
}

static void	print_port_overflow_error(t_traceroute *traceroute) {
	int total_probes;
	int max_base_port;

	total_probes = traceroute->options.max_hops * traceroute->options.probes_per_hop;
	max_base_port = 65535 - (total_probes - 1);
	fprintf(stderr, "ft_traceroute: invalid -p with current -m/-q values\n");
	fprintf(stderr, "  current: -m %d, -q %d, -p %d\n",
		traceroute->options.max_hops,
		traceroute->options.probes_per_hop,
		traceroute->options.base_port);
	fprintf(stderr, "  reason : we send %d probes, so the last port is p + %d\n",
		total_probes, total_probes - 1);
	fprintf(stderr, "  rule   : last port must be <= 65535\n");
	fprintf(stderr, "  fix    : set -p <= %d, or lower -m / -q\n", max_base_port);
}

int parse_arguments(int argc, char **argv, t_traceroute *traceroute) {
	int i;

	traceroute->options.help = 0;
	traceroute->options.max_hops = 30;
	traceroute->options.probes_per_hop = 3;
	traceroute->options.timeout_ms = 5000;
	traceroute->options.base_port = TR_UDP_BASE_PORT;
	traceroute->target_host = NULL;

	for (i = 1; i < argc; i++) {
		if (ft_strcmp(argv[i], "--help") == 0) {
			traceroute->options.help = 1;
		} else if (ft_strcmp(argv[i], "-m") == 0) {
			int value;

			if (!parse_option_value(argc, argv, &i, &value, "-m"))
				return 1;
			if (value < 1 || value > 255) {
				fprintf(stderr, "ft_traceroute: -m must be between 1 and 255\n");
				return 1;
			}
			traceroute->options.max_hops = value;
		} else if (ft_strcmp(argv[i], "-q") == 0) {
			int value;

			if (!parse_option_value(argc, argv, &i, &value, "-q"))
				return 1;
			if (value < 1 || value > TR_MAX_PROBES) {
				fprintf(stderr, "ft_traceroute: -q must be between 1 and %d\n", TR_MAX_PROBES);
				return 1;
			}
			traceroute->options.probes_per_hop = value;
		} else if (ft_strcmp(argv[i], "-w") == 0) {
			int value;

			if (!parse_option_value(argc, argv, &i, &value, "-w"))
				return 1;
			if (value < 1 || value > 60) {
				fprintf(stderr, "ft_traceroute: -w must be between 1 and 60 seconds\n");
				return 1;
			}
			traceroute->options.timeout_ms = value * 1000;
		} else if (ft_strcmp(argv[i], "-p") == 0) {
			int value;

			if (!parse_option_value(argc, argv, &i, &value, "-p"))
				return 1;
			if (value < 1 || value > 65535) {
				fprintf(stderr, "ft_traceroute: -p must be between 1 and 65535\n");
				return 1;
			}
			traceroute->options.base_port = value;
		} else if (argv[i][0] ==  '-'){
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
	if (traceroute->options.base_port + (traceroute->options.max_hops * traceroute->options.probes_per_hop) - 1 > 65535) {
		print_port_overflow_error(traceroute);
		return 1;
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
	printf("  --help         give this help list\n");
	printf("  -m             set max hop count (1..255)\n");
	printf("  -q             set number of probes per hop (1..%d)\n", TR_MAX_PROBES);
	printf("  -w             set timeout per probe in seconds (1..60)\n");
	printf("  -p             set UDP base destination port (1..65535)\n\n");
}

void print_help(char *prog_name) {
	print_usage(prog_name);
}