#pragma once
#include <string>

using namespace std;

bool safe_send(int sockfd, const string& message, int timeout_ms = 3000);
int connectToServer(const string &ip, int port, int timeout_ms = 3000);
bool recvLine(int sock, string &out, int timeout_ms = 3000);
