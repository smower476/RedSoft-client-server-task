#pragma once

#include <atomic>
#include <set>
#include <mutex>
#include <string>
#include <memory>

struct Channel;  

class ClientHandler {
public:
    ClientHandler(int client_fd,
                  const std::atomic<bool>& stop_flag,
                  std::mutex& client_sockets_mutex,
                  std::set<int>& client_sockets);
    
    void operator()();  
    
private:
    int client_fd_;
    const std::atomic<bool>& stop_flag_;
    std::mutex& client_sockets_mutex_;
    std::set<int>& client_sockets_;

    void process_command(const std::string& command);
    void handleJoin(Channel& ch, const std::string& nick);
    void handleExit(Channel& ch, const std::string& nick);
    void handleSend(Channel& ch, const std::string& nick, const std::string& message);
    void handleRead(Channel& ch, const std::string& nick);

    std::shared_ptr<Channel> GetOrCreateChannel(const std::string& channel_name, const std::string& action);
};
