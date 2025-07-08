#include "../include/connection.h"
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <poll.h>
#include <fcntl.h>
#include <errno.h>
#include <cstring>

using namespace std;

bool safe_send(int sockfd, const string& message, int timeout_ms) {
    const char* data = message.c_str();
    size_t total_sent = 0;
    size_t to_send = message.size();

    while (total_sent < to_send) {
        pollfd pfd{sockfd, POLLOUT, 0};
        int res = poll(&pfd, 1, timeout_ms);
        if (res <= 0) {
            if (res == 0) cerr << "safe_send: timeout" << endl;
            else perror("poll");
            return false;
        }

        ssize_t sent = send(sockfd, data + total_sent, to_send - total_sent, MSG_NOSIGNAL);
        if (sent < 0) {
            if (errno == EINTR) continue;

            if (errno == EPIPE || errno == ECONNRESET || errno == ENOTCONN ||
                errno == ETIMEDOUT || errno == EHOSTUNREACH) {
                cerr << "safe_send: соединение разорвано: " << strerror(errno) << endl;
            } else {
                perror("send");
            }

            return false;
        }

        if (sent == 0) {
            cerr << "safe_send: соединение закрыто" << endl;
            return false;
        }

        total_sent += sent;
    }

    return true;
}


int connectToServer(const string &ip, int port, int timeout_ms) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        return -1;
    }

    int flags = fcntl(sock, F_GETFL, 0);
    if (flags < 0 || fcntl(sock, F_SETFL, flags | O_NONBLOCK) < 0) {
        perror("fcntl");
        close(sock);
        return -1;
    }

    sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip.c_str(), &serv_addr.sin_addr) <= 0) {
        cerr << "Неверный адрес сервера" << endl;
        close(sock);
        return -1;
    }

    int res = connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    if (res < 0) {
        if (errno != EINPROGRESS) {
            perror("connect");
            close(sock);
            return -1;
        }

        
        pollfd pfd{};
        pfd.fd = sock;
        pfd.events = POLLOUT;

        int poll_res = poll(&pfd, 1, timeout_ms);
        if (poll_res <= 0) {
            if (poll_res == 0) cerr << "connect: timeout" << endl;
            else perror("poll");
            close(sock);
            return -1;
        }

        int err = 0;
        socklen_t len = sizeof(err);
        if (getsockopt(sock, SOL_SOCKET, SO_ERROR, &err, &len) < 0 || err != 0) {
            cerr << "connect failed: " << strerror(err) << endl;
            close(sock);
            return -1;
        }
    }

    if (fcntl(sock, F_SETFL, flags) < 0) {
        perror("fcntl restore");
        close(sock);
        return -1;
    }

    return sock;
}

bool recvLine(int sock, std::string &out, int timeout_ms) {
    out.clear();
    char c;

    while (true) {
        pollfd pfd{sock, POLLIN, 0};
        int res = poll(&pfd, 1, timeout_ms);
        if (res <= 0) {
            if (res == 0) {
                std::cerr << "recvLine: timeout\n";
            } else {
                std::cerr << "recvLine: poll error: " << std::strerror(errno) << "\n";
            }
            return false;
        }

        ssize_t r = recv(sock, &c, 1, 0);
        if (r < 0) {
            if (errno == EINTR) continue;
            std::cerr << "recvLine: recv error: " << std::strerror(errno) << "\n";
            return false;
        }
        if (r == 0) {
            std::cerr << "recvLine: connection closed by peer\n";
            return false;
        }

        if (c == '\n') break;
        if (c != '\r') out.push_back(c);
    }

    return true;
}

