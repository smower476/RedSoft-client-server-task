#pragma once
#include <string>

int connectToServer(const std::string &ip, int port, int timeout_ms = 3000);
