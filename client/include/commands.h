#pragma once
#include <string>
#include <sstream>

bool handleSend(int sock, const std::string &channel, const std::string &nick, std::istringstream &iss);
bool handleRead(int sock, const std::string &channel, const std::string &nick);
bool handleJoin(int sock, std::string &channel, const std::string &nick, std::istringstream &iss);
bool handleExit(int sock, const std::string &channel, const std::string &nick);
void commandLoop(int sock, std::string &channel, const std::string &nick);




