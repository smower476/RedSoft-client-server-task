#pragma once
#include <string>
#include <sstream>

#define RECONNECT_ATTEMPTS 1

#ifndef MAX_LINE_LENGTH
#define MAX_LINE_LENGTH 1024
#endif

class CommandHandler {
private:
    int sock;
    std::string server_ip;
    int server_port;
    std::string channel;
    std::string nick;
    std::string read_buf;  
    size_t read_pos = 0;   

    bool handleSend(std::istringstream &iss);
    bool handleRead();
    bool handleJoin(std::istringstream &iss);
    bool handleExit();
    bool tryReconnect();
    std::string recvMessage();  
public:
    CommandHandler(const std::string &ip, int port, int sock, 
                  const std::string &channel, const std::string &nick);
    void run();
};
