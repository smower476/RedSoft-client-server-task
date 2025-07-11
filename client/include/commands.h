#pragma once
#include <string>
#include <sstream>

#define RECONNECT_ATTEMPTS 1

class CommandHandler {
    int sock;
    std::string server_ip;
    int server_port;
    std::string channel;
    std::string nick;

public:
    CommandHandler(const std::string &ip, int port, int sock, const std::string &channel, const std::string &nick);
    void run();
    bool handleSend(std::istringstream &iss);
    bool handleRead();
    bool handleJoin(std::istringstream &iss);
    bool handleExit();
private:
    bool tryReconnect();
};
