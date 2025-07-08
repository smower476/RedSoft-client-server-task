#include <connections.h>
#include <netinet/in.h>
#include <poll.h>
#include <unistd.h>
#include <csignal>

std::mutex client_sockets_mutex;
std::set<int> client_sockets;
int server_fd = -1;
std::atomic<bool> stopFlag{false};

void shutdown_server() {
    stopFlag = true;
    
    if (server_fd != -1) {
        shutdown(server_fd, SHUT_RDWR);
    }
}
void signal_handler(int signum) {
    if (signum == SIGINT) {
        shutdown_server();
    }
}
