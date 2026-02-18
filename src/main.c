#include "../include/ft_traceroute.h"

static t_ping *g_ping = NULL;

void signal_handler(int sig) {
    (void)sig;
    if (g_ping != NULL) {
        display_stats(g_ping);
        if (g_ping->socket_fd > 0)
            close(g_ping->socket_fd);
        exit(0);
    }
}

int main(int argc, char **argv) {
    t_ping ping;
    int ret;
    char ip_str[INET_ADDRSTRLEN];
    struct timeval send_time, timeout, start_time, current_time;
    double rtt;
    int ttl;
    int deadline_reached = 0;
    
    memset(&ping, 0, sizeof(t_ping));
    ping.pid = getpid();
    ping.sequence = 0;
    ping.stats.min_rtt = -1;
    g_ping = &ping;

    signal(SIGINT, signal_handler);
    
    ret = parse_arguments(argc, argv, &ping);
    if (ret == 1)
        return 1;
    if (ret == 2)
        return 0;
    
    if (resolve_hostname(&ping) != 0)
        return 1;
    
    inet_ntop(AF_INET, &ping.target_addr.sin_addr, ip_str, INET_ADDRSTRLEN);
    int total_size = 8 + ping.options.packet_size;
    printf("PING %s (%s) %d(%d) bytes of data.\n", ping.target_host, ip_str, ping.options.packet_size, total_size + 20);
    
    if (create_socket(&ping) != 0) {
        fprintf(stderr, "ft_ping: socket creation failed (need root)\n");
        return 1;
    }
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    if (setsockopt(ping.socket_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        perror("ft_ping: setsockopt: SO_RCVTIMEO");
    }

    if (ping.options.deadline > 0) {
        gettimeofday(&start_time, NULL);
    }
    
    while (1) {
        if (ping.options.count > 0 && ping.stats.packets_send >= (uint16_t)ping.options.count) {
            break;
        }
        if (ping.options.deadline > 0) {
            gettimeofday(&current_time, NULL);
            double elapsed_time = (current_time.tv_sec - start_time.tv_sec) + (current_time.tv_usec - start_time.tv_usec) / 1000000.0;
            if (elapsed_time >= ping.options.deadline) {
                deadline_reached = 1;
                break;
            }
        }
        gettimeofday(&send_time, NULL);
        if (send_packet(&ping) != 0)
        {
            fprintf(stderr, "ft_ping: failed to send packet\n");
            break;
        }

        ret = receive_packet(&ping, &send_time, &rtt, &ttl);
        if (ret == 0) {
            display_packet_received(&ping, rtt, ttl);
        } else if (ret == -1) {
            if (!ping.options.quiet) {
                printf("Request timeout for icmp_seq %d\n", ping.sequence);
            }
        }
        ping.sequence++;
        sleep(1);
    }
    if (deadline_reached) {
        display_stats(&ping);
    } else {
        display_stats(&ping);
    }
    close(ping.socket_fd);
    return 0;
}