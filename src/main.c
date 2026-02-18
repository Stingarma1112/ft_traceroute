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
    t_traceroute traceroute;
    
}