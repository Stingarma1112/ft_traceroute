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

char *resolve_router_hostname(struct sockaddr_in *addr) {
	static char hostname[NI_MAXHOST];
	char ip_str[INET_ADDRSTRLEN];
	int ret;

	inet_ntop(AF_INET, &addr->sin_addr, ip_str, INET_ADDRSTRLEN);

	ret = getnameinfo((struct sockaddr *)addr, sizeof(struct sockaddr_in), hostname, NI_MAXHOST, NULL, 0, NI_NAMEREQD);
	if (ret == 0 && hostname[0] != '\0') {
		char *result = malloc(strlen(hostname) + 1);
		if (result)
			ft_strcpy(result, hostname);
		return result;
	}

	struct hostent *host = gethostbyaddr(&addr->sin_addr, sizeof(addr->sin_addr), AF_INET);
	if (host && host->h_name) {
		char *result = malloc(strlen(host->h_name) + 1);
		if (!result)
			return NULL;
		if (result)
			ft_strcpy(result, host->h_name);
		return result;
	}
	return NULL;
}