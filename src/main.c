#include "../include/ft_traceroute.h"

static t_traceroute *g_traceroute = NULL;

void signal_handler(int sig) {
    (void)sig;
    if (g_traceroute != NULL) {
        display_stats(g_traceroute);
        if (g_traceroute->hops != NULL)
            free(g_traceroute->hops);
        if (g_traceroute->socket_fd > 0)
            close(g_traceroute->socket_fd);
        exit(0);
    }
}

int main(int argc, char **argv) {
    t_traceroute traceroute;

    ft_bzero(&traceroute, sizeof(t_traceroute));
    traceroute.pid = getpid();
    traceroute.sequence = 0;
    traceroute.stats.current_hop = 0;
    g_traceroute = &traceroute;
    signal(SIGINT, signal_handler);

    int ret = parse_arguments(argc, argv, &traceroute);
    if (ret == 1)
        return 1;
    if (ret == 2)
        return 0;

    if (resolve_hostname(&traceroute) != 0)
        return 1;
    if (create_socket(&traceroute) != 0) {
        fprintf(stderr, "ft_traceroute: socket creation failed (need root privileges)\n");
        return 1;
    }
    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    if (setsockopt(traceroute.socket_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        perror("ft_traceroute: setsockopt: SO_RCVTIMEO");
    }

    traceroute.max_hops = traceroute.options.max_hops;
    traceroute.hops = malloc(traceroute.max_hops * sizeof(t_hop));
    if (!traceroute.hops) {
        perror("ft_traceroute: error allocating memory for hops");
        return 1;
    }
    for (int i = 0; i < traceroute.max_hops; i++) {
        ft_bzero(&traceroute.hops[i], sizeof(t_hop));
        for (int j = 0; j < 3; j++) {
            traceroute.hops[i].rtt[j] = -1.0;
            traceroute.hops[i].hostname[j] = NULL;
        }
    }
    display_header(&traceroute);

    int ttl;
    int probe;
    int destination_reached = 0;
    struct timeval send_time;
    double rtt;
    struct sockaddr_in router_addr;

    for (ttl = 1; ttl <= traceroute.options.max_hops; ttl++) {
        traceroute.stats.current_hop = ttl;
        for (probe = 0; probe < traceroute.options.probes_per_hop; probe++) {
            gettimeofday(&send_time, NULL);
            if (send_packet_with_ttl(&traceroute, ttl) != 0) {
                fprintf(stderr, "ft_traceroute: failed to send packet\n");
                continue;
            }
            int recv_result = receive_packet_for_hop(&traceroute, ttl, &send_time, &rtt, &router_addr);
            if (recv_result == 0) {
               traceroute.hops[ttl - 1].router_addr[probe] = router_addr;
               traceroute.hops[ttl - 1].rtt[probe] = rtt;
               traceroute.hops[ttl - 1].received_count++;
            } else if (recv_result == -1) {
                //timeout
            } else if (recv_result == 1) {
                traceroute.hops[ttl - 1].router_addr[probe] = router_addr;
                traceroute.hops[ttl - 1].rtt[probe] = rtt;
                traceroute.hops[ttl - 1].received_count++;
                traceroute.hops[ttl - 1].is_destination = 1;
                destination_reached = 1;
                break;
            } else if (recv_result == 2) {
                traceroute.hops[ttl - 1].router_addr[probe] = router_addr;
                traceroute.hops[ttl - 1].rtt[probe] = rtt;
                traceroute.hops[ttl - 1].received_count++;
                traceroute.hops[ttl - 1].is_destination = 1;
                destination_reached = 1;
                break;
            }
        }
        display_hop(&traceroute, ttl);
        if (destination_reached)
            break;
    }
    free(traceroute.hops);
    return 0;
}