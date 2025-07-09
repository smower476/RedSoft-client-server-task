#pragma once
#include <string>
#include <sstream>

class CommandHandler {
private:
    int sock;
    std::string channel;
    std::string nick;

    bool handleSend(std::istringstream& iss);
    bool handleRead();
    bool handleJoin(std::istringstream& iss);
    bool handleExit();

public:
    CommandHandler(int sock, const std::string& channel, const std::string& nick);
    void run();
};
