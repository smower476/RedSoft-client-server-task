#pragma once

#include <atomic>
#include <set>
#include <mutex>
#include <string>

void handle_client(
     int client_fd, 
     const std::atomic<bool> &stopFlag, 
     std::mutex &client_sockets_mutex, 
     std::set<int> &client_sockets);

struct Channel;
void handle_join(int client_fd, Channel &ch, const std::string& nick);
void handle_exit(int client_fd, Channel &ch, const std::string& nick);
void handle_send(int client_fd, Channel &ch, const std::string& nick, const std::string& message);
void handle_read(int client_fd, Channel &ch, const std::string& nick);
