#include "../include/ft_traceroute.h"

static t_traceroute *g_traceroute = NULL;

static void cleanup_traceroute(t_traceroute *traceroute) {
    int probes;

    if (traceroute == NULL)
        return;
    probes = traceroute->options.probes_per_hop;
    if (probes <= 0 || probes > TR_MAX_PROBES)
        probes = TR_MAX_PROBES;
    if (traceroute->hops != NULL) {
        for (int i = 0; i < traceroute->max_hops; i++) {
            for (int j = 0; j < probes; j++) {
                if (traceroute->hops[i].hostname[j] != NULL)
                    free(traceroute->hops[i].hostname[j]);
            }
        }
        free(traceroute->hops);
        traceroute->hops = NULL;
    }
    if (traceroute->socket_fd >= 0) {
        close(traceroute->socket_fd);
        traceroute->socket_fd = -1;
    }
    if (traceroute->send_socket_fd >= 0) {
        close(traceroute->send_socket_fd);
        traceroute->send_socket_fd = -1;
    }
}

void signal_handler(int sig) {
    (void)sig;
    if (g_traceroute != NULL) {
        display_stats(g_traceroute);
        cleanup_traceroute(g_traceroute);
        exit(0);
    }
}

int main(int argc, char **argv) {
    t_traceroute traceroute;

    ft_bzero(&traceroute, sizeof(t_traceroute));
    traceroute.pid = getpid();
    traceroute.sequence = 0;
    traceroute.stats.current_hop = 0;
    traceroute.socket_fd = -1;
    traceroute.send_socket_fd = -1;
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
    timeout.tv_sec = traceroute.options.timeout_ms / 1000;
    timeout.tv_usec = (traceroute.options.timeout_ms % 1000) * 1000;
    if (setsockopt(traceroute.socket_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        perror("ft_traceroute: setsockopt: SO_RCVTIMEO");
    }

    traceroute.max_hops = traceroute.options.max_hops;
    traceroute.hops = malloc(traceroute.max_hops * sizeof(t_hop));
    if (!traceroute.hops) {
        perror("ft_traceroute: error allocating memory for hops");
        cleanup_traceroute(&traceroute);
        return 1;
    }
    ft_bzero(traceroute.hops, traceroute.max_hops * sizeof(t_hop));
    for (int i = 0; i < traceroute.max_hops; i++) {
        for (int j = 0; j < traceroute.options.probes_per_hop; j++) {
            traceroute.hops[i].rtt[j] = -1.0;
        }
    }
    display_header(&traceroute);

    int ttl;
    int probe;
    int destination_reached = 0;
    struct timeval send_time;
    struct timeval hop_start_time;
    double rtt;
    struct sockaddr_in router_addr;

    for (ttl = 1; ttl <= traceroute.options.max_hops; ttl++) {
        traceroute.stats.current_hop = ttl;
        gettimeofday(&hop_start_time, NULL);
        for (probe = 0; probe < traceroute.options.probes_per_hop; probe++) {
            uint16_t expected_seq;

            gettimeofday(&send_time, NULL);
            expected_seq = traceroute.sequence;
            if (send_packet_with_ttl(&traceroute, ttl) != 0) {
                fprintf(stderr, "ft_traceroute: failed to send packet\n");
                continue;
            }
            int recv_result = receive_packet_for_hop(&traceroute, ttl, &send_time, &hop_start_time, &rtt, &router_addr, expected_seq);
            if (recv_result == 0) {
               traceroute.hops[ttl - 1].router_addr[probe] = router_addr;
               traceroute.hops[ttl - 1].rtt[probe] = rtt;
               traceroute.hops[ttl - 1].received_count++;
               traceroute.hops[ttl - 1].hostname[probe] = resolve_router_hostname(&router_addr);
            } else if (recv_result == -1) {
                //timeout
            } else if (recv_result == 1) {
                traceroute.hops[ttl - 1].router_addr[probe] = router_addr;
                traceroute.hops[ttl - 1].rtt[probe] = rtt;
                traceroute.hops[ttl - 1].received_count++;
                traceroute.hops[ttl - 1].hostname[probe] = resolve_router_hostname(&router_addr);
                traceroute.hops[ttl - 1].is_destination = 1;
                destination_reached = 1;
            } else if (recv_result == 2) {
                traceroute.hops[ttl - 1].router_addr[probe] = router_addr;
                traceroute.hops[ttl - 1].rtt[probe] = rtt;
                traceroute.hops[ttl - 1].received_count++;
                traceroute.hops[ttl - 1].hostname[probe] = resolve_router_hostname(&router_addr);
                traceroute.hops[ttl - 1].is_destination = 1;
                destination_reached = 1;
            }
        }
        display_hop(&traceroute, ttl);
        if (destination_reached)
            break;
    }
    cleanup_traceroute(&traceroute);
    return 0;
}