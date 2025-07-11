#pragma once

#include <atomic>
#include <set>
#include <mutex>
#include <string>
#include <memory>
#include <map>
#include <deque>

#define MAX_MESSAGE_LENGTH 256

struct Message {
    std::string nick;
    std::string text;
    Message(const std::string& n, const std::string& t) : nick(n), text(t) {}
};

struct Channel {
    std::deque<Message> messages;
    std::set<std::string> members;
    std::mutex mtx;
};


class ClientHandler {
public:
    ClientHandler(int client_fd,
                  const std::atomic<bool>& stop_flag,
                  std::mutex& client_sockets_mutex,
                  std::set<int>& client_sockets);
    
    void operator()();  
    
private:
    int client_fd;
    const std::atomic<bool>& stop_flag;
    std::mutex& client_sockets_mutex;
    std::set<int>& client_sockets;
    static std::mutex channels_mutex;
    static std::map<std::string, std::shared_ptr<Channel>> channels;
    
    void processCommand(const std::string& command);
    void handleJoin(Channel& ch, const std::string& nick);
    void handleExit(Channel& ch, const std::string& nick);
    void handleSend(Channel& ch, const std::string& nick, const std::string& message);
    void handleRead(Channel& ch, const std::string& nick);

    std::shared_ptr<Channel> getOrCreateChannel(const std::string& channel_name, const std::string& action);
};
