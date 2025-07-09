#include <iostream>
#include <string>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <cstring>
#include <network_utils.h>

bool safe_send(int sockfd, const std::string& message, int timeout_ms) {
    const char* data = message.c_str();
    size_t total_sent = 0;
    size_t to_send = message.size();

    while (total_sent < to_send) {
        pollfd pfd{sockfd, POLLOUT, 0};
        int res = poll(&pfd, 1, timeout_ms);
        if (res <= 0) {
            if (res == 0) {
                std::cerr << "safe_send: timeout" << std::endl;
            } else {
                std::cerr << "safe_send: poll error: " << std::strerror(errno) << std::endl;
            }
            return false;
        }

        ssize_t sent = send(sockfd, data + total_sent, to_send - total_sent, MSG_NOSIGNAL);
        if (sent < 0) {
            if (errno == EINTR) {
                continue;
            }

            if (errno == EPIPE || errno == ECONNRESET || errno == ENOTCONN ||
                errno == ETIMEDOUT || errno == EHOSTUNREACH) {
                std::cerr << "safe_send: connection broken: " << std::strerror(errno) << std::endl;
            } else {
                std::cerr << "safe_send: send error: " << std::strerror(errno) << std::endl;
            }
            return false;
        }

        if (sent == 0) {
            std::cerr << "safe_send: connection closed by peer" << std::endl;
            return false;
        }

        total_sent += static_cast<size_t>(sent);
    }

    return true;
}

bool recv_line(int sock, std::string& out, int timeout_ms) {
    out.clear();
    bool overflow = false;
    char c;

    while (true) {
        pollfd pfd = {sock, POLLIN, 0};
        int res = poll(&pfd, 1, timeout_ms);
        
        if (res == 0) {
            std::cerr << "recv_line: timeout" << std::endl;
            return false; 
        }
        if (res < 0) {
            if (errno == EINTR) continue;
            perror("poll");
            return false;
        }

        ssize_t r = recv(sock, &c, 1, 0);
        if (r < 0) {
            if (errno == EINTR) continue;
            perror("recv");
            return false;
        }
        if (r == 0) {
            std::cerr << "recv_line: connection closed by peer" << std::endl;
            return false;
        }

        if (c == '\n') {
            break;
        }

        if (c == '\r') {
            continue;
        }

        if (out.size() < MAX_COMMAND_LEN) {
            out.push_back(c);
        } else if (!overflow) {
            overflow = true;
            std::cerr << "recv_line: command exceeds max length" << std::endl;
        }
    }

    if (overflow) {
        std::cerr << "Command truncated to " << MAX_COMMAND_LEN << " bytes" << std::endl;
        if (out.size() > MAX_COMMAND_LEN) {
            out.resize(MAX_COMMAND_LEN);
        }
    }

    return true;
}
