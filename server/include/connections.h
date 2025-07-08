#pragma once

#include <atomic>
#include <mutex>
#include <set>

extern std::mutex client_sockets_mutex;
extern std::set<int> client_sockets;
extern int server_fd;
extern std::atomic<bool> stopFlag;

void shutdown_server();
void signal_handler(int signum);
