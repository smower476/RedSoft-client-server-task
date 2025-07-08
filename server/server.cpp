#include <commands.h>
#include <csignal>
#include <iostream>
#include <mutex>
#include <netinet/in.h>
#include <thread>
#include <unistd.h>
#include <vector>
#include <algorithm>
#include <cstring>
#include <set>
#include <atomic>

std::vector<std::thread> threads;
std::mutex threads_mutex;

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


int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <port>\n";
        return 1;
    }

    int port;
    try {
        port = std::stoi(argv[1]);
    } catch (std::exception&) {
        std::cerr << "ERROR: invalid port\n";
        return 1;
    }

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGINT, &sa, nullptr) < 0) {
        perror("sigaction");
        return 1;
    }

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        return 1;
    }
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    if (bind(server_fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(server_fd);
        return 1;
    }
    if (listen(server_fd, 10) < 0) {
        perror("listen");
        close(server_fd);
        return 1;
    }
    std::cout << "Server listening on port " << port << std::endl;

    while (!stopFlag) {
        sockaddr_in client_addr;
        socklen_t len = sizeof(client_addr);
        int client_fd = accept(server_fd, (sockaddr*)&client_addr, &len);
        if (client_fd < 0) {
            if (stopFlag || errno == EINTR) break;
            perror("accept");
            continue;
        }

        {
            std::lock_guard<std::mutex> lock(client_sockets_mutex);
            client_sockets.insert(client_fd);
        }

        {
            std::lock_guard<std::mutex> lock(threads_mutex);
            threads.erase(
                std::remove_if(threads.begin(), threads.end(),
                    [](std::thread& t) { return !t.joinable(); }),
                threads.end());

            threads.emplace_back([client_fd]() {
                handle_client(client_fd, stopFlag, client_sockets_mutex, client_sockets);
            });
        }
    }

    if (server_fd != -1) {
        close(server_fd);
        server_fd = -1;
    }

    {
        std::lock_guard<std::mutex> lock(client_sockets_mutex);
        for (int fd : client_sockets) {
            shutdown(fd, SHUT_RDWR); 
        }
    }

    {
        std::lock_guard<std::mutex> lock(threads_mutex);
        for (auto& t : threads) {
            if (t.joinable()) t.join();
        }
    }

    {
        std::lock_guard<std::mutex> lock(client_sockets_mutex);
        for (int fd : client_sockets) {
            close(fd);
        }
        client_sockets.clear();
    }

    std::cout << "Server shutdown complete.\n";
    return 0;
}
