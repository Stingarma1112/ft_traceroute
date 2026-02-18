#include "../include/ft_ping.h"

int parse_arguments(int argc, char **argv, t_ping *ping) {
	int i;
	ping->options.verbose = 0;
	ping->options.help = 0;
	ping->options.quiet = 0;
	ping->options.count = -1;
	ping->options.deadline = -1;
	ping->options.ttl_value = -1;
	ping->options.packet_size = 64;
	ping->target_host = NULL;

	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-v") == 0) {
			ping->options.verbose = 1;
		} else if (strcmp(argv[i], "-?") == 0 || strcmp(argv[i], "--help") == 0) {
			ping->options.help = 1;
		} else if (strcmp(argv[i], "-q") == 0) {
			ping->options.quiet = 1;
		} else if (strcmp(argv[i], "-c") == 0) {
			int j;

			if (i + 1 >= argc) {
				fprintf(stderr, "ft_ping: option requires an argument -- 'c'\n");
				return 1;
			}
			for (j = 0; argv[i + 1][j] != '\0'; j++) {
				if (argv[i + 1][j] < '0' || argv[i + 1][j] > '9') {
					fprintf(stderr, "ft_ping: invalid argument: '%s'\n", argv[i + 1]);
					return 1;
				}
			}
			ping->options.count = atoi(argv[++i]);
			if (ping->options.count <= 0) {
				fprintf(stderr, "ft_ping: invalid count value\n");
				return 1;
			}
		} else if (strcmp(argv[i], "-w") == 0) {
			if (i + 1 >= argc) {
				fprintf(stderr, "ft_ping: option requires an argument -- 'w'\n");
				return 1;
			}
			ping->options.deadline = atoi(argv[++i]);
			if (ping->options.deadline <= 0) {
				fprintf(stderr, "ft_ping: invalid deadline value\n");
				return 1;
			}
		} else if (strcmp(argv[i], "-ttl") == 0) {
			if (i + 1 >= argc) {
				fprintf(stderr, "ft_ping: option requires an argument -- 'ttl'\n");
				return 1;
			}
			ping->options.ttl_value = atoi(argv[++i]);
			if (ping->options.ttl_value < 1 || ping->options.ttl_value > 255) {
				fprintf(stderr, "ft_ping: invalid TTL value (1-255)\n");
				return 1;
			}
		} else if (strcmp(argv[i], "-s") == 0) {
			if (i + 1 >= argc) {
				fprintf(stderr, "ft_ping: option requires an argument -- 's'\n");
				return 1;
			}
			ping->options.packet_size = atoi(argv[++i]);
			if (ping->options.packet_size < 0 || ping->options.packet_size > 65535) {
				fprintf(stderr, "ft_ping: invalid packet size (0-65535)\n");
				return 1;
			}
		} else if (argv[i][0] == '-') {
			fprintf(stderr, "ft_ping: invalid option -- '%c'\n", argv[i][1]);
			fprintf(stderr, "Try 'ft_ping -?' for more information.\n");
			return 1;
		} else {
			if (ping->target_host != NULL) {
				fprintf(stderr, "ft_ping: only one hostname allowed\n");
				return 1;
			}
			ping->target_host = argv[i];
		}
	}
	if (ping->options.help) {
		print_help(argv[0]);
		return 2;
	}
	if (ping->target_host == NULL) {
		fprintf(stderr, "ft_ping: missing operand\n");
		fprintf(stderr, "Try 'ft_ping -?' for more information.\n");
		return 1;
	}
	return 0;
}

void print_usage(char *prog_name) {
	printf("Usage: %s [OPTION...] HOST ...\n", prog_name);
	printf("Send ICMP ECHO_REQUEST packets to network hosts\n");
	printf("  -v                 verbose output\n");
	printf("  -q                 quiet output\n");
	printf("  -c                 send specified number of ECHO_REQUEST packets\n");
	printf("  -w                 wait for seconds\n");
	printf("  -ttl               set TTL\n");
	printf("  -s                 send packets with specified size\n");
	printf("  -?, --help         give this help list\n\n");
	printf("Report bugs to <bug-inetutils@gnu.org>\n");
}

void print_help(char *prog_name) {
	print_usage(prog_name);
}