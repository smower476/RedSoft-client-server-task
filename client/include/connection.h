#pragma once
#include <string>

int connect_to_server(const std::string &ip, int port, int timeout_ms = 3000);
