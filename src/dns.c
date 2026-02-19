#include "../include/ft_traceroute.h"

int resolve_hostname(t_traceroute *traceroute) {
	struct addrinfo hints;
	struct addrinfo *results;
	struct addrinfo *rp;
	int ret;

	ft_memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_RAW;
	hints.ai_protocol = IPPROTO_ICMP;
	hints.ai_flags = 0;

	ret = getaddrinfo(traceroute->target_host, NULL, &hints, &results);
	if (ret != 0) {
		fprintf(stderr, "ft_traceroute: %s: %s\n", traceroute->target_host, gai_strerror(ret));
		return 1;
	}
	rp = results;
	while (rp != NULL) {
		if (rp->ai_family == AF_INET) {
			ft_memcpy(&traceroute->target_addr, rp->ai_addr, sizeof(struct sockaddr_in));
			break;
		}
		rp = rp->ai_next;
	}

	freeaddrinfo(results);

	if (rp == NULL) {
		fprintf(stderr, "ft_traceroute: %s: Name or service not known\n", traceroute->target_host);
		return 1;
	}
	return 0;
}