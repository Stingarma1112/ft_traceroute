#include "../include/ft_ping.h"

int resolve_hostname(t_ping * ping) {
	struct addrinfo hints;
	struct addrinfo *results;
	struct addrinfo *rp;
	int ret;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_RAW;
	hints.ai_protocol = IPPROTO_ICMP;
	hints.ai_flags = 0;

	ret = getaddrinfo(ping->target_host, NULL, &hints, &results);
	if (ret != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(ret));
		return 1;
	}

	for (rp = results; rp != NULL; rp = rp->ai_next) {
		if (rp->ai_family == AF_INET) {
			memcpy(&ping->target_addr, rp->ai_addr, sizeof(struct sockaddr_in));
			break;
		}
	}
	freeaddrinfo(results);
	if (rp == NULL) {
		fprintf(stderr, "ft_ping: %s: Name or service not known\n", ping->target_host);
		return 1;
	}
	return 0;
}