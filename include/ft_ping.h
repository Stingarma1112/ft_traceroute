#ifndef FT_PING_H
# define FT_PING_H

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
	int verbose;
	int help;
	int quiet;
	int count;
	int deadline;
	int ttl_value;
	int packet_size;
} t_options;


typedef struct s_stats {
	unsigned int packets_send;
	unsigned int packets_received;
	double min_rtt;
	double max_rtt;
	double total_rtt;
	double total_rtt_squared;
	unsigned int errors;
} t_stats;

typedef struct s_ping {
	t_options options;
	t_stats stats;
	char *target_host;
	struct sockaddr_in target_addr;
	int socket_fd;
	uint16_t sequence;
	pid_t pid;
} t_ping;

//=======================================//
//==================PARSING==============//
//=======================================//

int parse_arguments(int argc, char **arv, t_ping *ping);
void print_usage(char *prog_name);
void print_help(char *prog_name);

//=======================================//
//==================DNS==================//
//=======================================//

int resolve_hostname(t_ping * ping);

//=======================================//
//==================SOCKET===============//
//=======================================//

int create_socket(t_ping *ping);

//=======================================//
//==================UTILS================//
//=======================================//

uint16_t calculate_checksum(void *data, int len);

//=======================================//
//==================ICMP=================//
//=======================================//

void build_icmp_packet(struct icmphdr *icmp, uint16_t sequence, pid_t pid, int packet_size);

//=======================================//
//==================SEND=================//
//=======================================//

int send_packet(t_ping *ping);

//=======================================//
//==================RECEIVE==============//
//=======================================//

int receive_packet(t_ping *ping, struct timeval *send_time, double *rtt, int *ttl);

//=======================================//
//==================DISPLAY==============//
//=======================================//

void display_packet_received(t_ping *ping, double rtt, int ttl);
void display_stats(t_ping *ping);

//=======================================//
//==================STATS================//
//=======================================//

void init_stats(t_stats *stats);
void update_rtt_stats(t_stats *stats, double rtt);

#endif