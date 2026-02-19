#ifndef FT_TRACEROUTE_H
# define FT_TRACEROUTE_H

# include <stdio.h>
# include <stdlib.h>
# include <stdint.h>
# include <string.h>
# include <unistd.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <netinet/ip.h>
# include <netinet/ip_icmp.h>
# include <arpa/inet.h>
# include <signal.h>
# include <sys/time.h>
# include <errno.h>
# include <netdb.h>

typedef struct s_options {
	int help;
	int max_hops;
	int probes_per_hop;
} t_options;


typedef struct s_stats {
	unsigned int packets_send;
	unsigned int packets_received;
	unsigned int errors;
	int current_hop;
} t_stats;

typedef struct s_hop {
	struct sockaddr_in router_addr;
	char *hostname;
	double rtt[3];
	int received_count;
	int is_destination;
} t_hop;

typedef struct s_traceroute {
	t_options options;
	t_stats stats;
	char *target_host;
	struct sockaddr_in target_addr;
	int socket_fd;
	uint16_t sequence;
	pid_t pid;
	t_hop *hops;
	int max_hops;
} t_traceroute;

//=======================================//
//==================PARSING==============//
//=======================================//

int parse_arguments(int argc, char **arv, t_traceroute *traceroute);
void print_usage(char *prog_name);
void print_help(char *prog_name);

//=======================================//
//==================DNS==================//
//=======================================//

int resolve_hostname(t_traceroute *traceroute);

//=======================================//
//==================SOCKET===============//
//=======================================//

int create_socket(t_traceroute *traceroute);

//=======================================//
//==================UTILS================//
//=======================================//

uint16_t calculate_checksum(void *data, int len);
int ft_strcmp(const char *s1, const char *s2);
void	*ft_memset(void *s, int c, size_t n);
void	ft_bzero(void *s, size_t n);

//=======================================//
//==================ICMP=================//
//=======================================//

void build_icmp_packet(struct icmphdr *icmp, uint16_t sequence, pid_t pid, int packet_size);

//=======================================//
//==================SEND=================//
//=======================================//

int send_packet_with_ttl(t_traceroute *traceroute, int ttl);

//=======================================//
//==================RECEIVE==============//
//=======================================//

int receive_packet_for_hop(t_traceroute *traceroute, int ttl, struct timeval *send_time, double *rtt, struct sockaddr_in *router_addr);

//=======================================//
//==================DISPLAY==============//
//=======================================//

void display_packet_received(t_traceroute *traceroute, double rtt, int ttl);
void display_stats(t_traceroute *traceroute);

//=======================================//
//==================STATS================//
//=======================================//

void init_stats(t_stats *stats);
void update_rtt_stats(t_stats *stats, double rtt);

#endif