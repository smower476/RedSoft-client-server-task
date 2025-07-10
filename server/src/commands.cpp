#include <network_utils.h>
#include <commands.h>
#include <validation.h>
#include <deque>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <sstream>
#include <vector>
#include <unistd.h>


ClientHandler::ClientHandler(int client_fd,
                             const std::atomic<bool> &stop_flag,
                             std::mutex &client_sockets_mutex,
                             std::set<int> &client_sockets)
    : client_fd(client_fd),
      stop_flag(stop_flag),
      client_sockets_mutex(client_sockets_mutex),
      client_sockets(client_sockets) {}

std::mutex ClientHandler::channels_mutex;
std::map<std::string, std::shared_ptr<Channel>> ClientHandler::channels;

void ClientHandler::operator()() {
    std::string line;
    while (!stop_flag && recv_line(client_fd, line)) {
        if (line.size() >= MAX_COMMAND_LEN) {
            safe_send(client_fd, "ERROR: command too long\n");
            continue; 
        }

        std::string cmd = trim(line);
        if (!cmd.empty()) {
            processCommand(cmd);
        }
    }
    
    close(client_fd);
    {
        std::lock_guard<std::mutex> lock(client_sockets_mutex);
        client_sockets.erase(client_fd);
    }
}

void ClientHandler::processCommand(const std::string& command) {
    std::istringstream iss(command);
    std::string action, channel_name, nick;
    iss >> action >> channel_name >> nick;
    
    if (action.empty() || channel_name.empty() || nick.empty()) {
        safe_send(client_fd, "ERROR: invalid command\n");
        return;
    }
    
    if (channel_name.size() > 24 || nick.size() > 24) {
        safe_send(client_fd, "ERROR: channel or nick too long\n");
        return;
    }

    auto ch_ptr = getOrCreateChannel(channel_name, action);
    if (!ch_ptr) {
        safe_send(client_fd, "ERROR: no such channel\n");
        return;
    }

    Channel& ch = *ch_ptr;

    if (action == "join") {
        handleJoin(ch, nick);
    } 
    else if (action == "exit") {
        handleExit(ch, nick);
    } 
    else if (action == "send") {
        std::string message;
        std::getline(iss, message);
        handleSend(ch, nick, trim(message));
    } 
    else if (action == "read") {
        handleRead(ch, nick);
    } 
    else {
        safe_send(client_fd, "ERROR: unknown command\n");
    }
}

std::shared_ptr<Channel> ClientHandler::getOrCreateChannel(
    const std::string& channel_name, const std::string& action) 
{
    std::lock_guard<std::mutex> lock(channels_mutex);
    auto it = channels.find(channel_name);
    
    if (it == channels.end()) {
        if (action == "send" || action == "join") {
            auto ch_ptr = std::make_shared<Channel>();
            channels[channel_name] = ch_ptr;
            return ch_ptr;
        }
        return nullptr;
    }
    return it->second;
}

void ClientHandler::handleJoin(Channel& ch, const std::string& nick) {
    std::string response;
    {
        std::lock_guard<std::mutex> lk(ch.mtx);
        if (!ch.members.insert(nick).second) {
            response = "ERROR: user already in channel\n";
        } else {
            response = "OK\n";
        }
    }
    safe_send(client_fd, response);
}

void ClientHandler::handleExit(Channel& ch, const std::string& nick) {
    std::string response;
    {
        std::lock_guard<std::mutex> lk(ch.mtx);
        if (!ch.members.erase(nick)) {
            response = "ERROR: not in channel\n";
        } else {
            response = "OK\n"; 
        }
    }
    safe_send(client_fd, response);
}

void ClientHandler::handleSend(Channel& ch, const std::string& nick, const std::string& message) {
    if (message.empty()) {
        safe_send(client_fd, "ERROR: message cannot be empty\n");
        return;
    }
    std::string response;

    std::string truncated = message;
    if (truncated.size() > 256) {
        response = "ERROR: message too long\n";
    }
    
    if (response.empty()) { 
        {
            std::lock_guard<std::mutex> lk(ch.mtx);
            if (!ch.members.count(nick)) {
                response = "ERROR: not in channel\n";
            } else {
                ch.messages.emplace_back(nick, truncated);
                if (ch.messages.size() > 40) {
                    ch.messages.pop_front();
                }
                response = "OK\n";
            }
        }
    }
    safe_send(client_fd, response);
}

void ClientHandler::handleRead(Channel& ch, const std::string& nick) {
    std::vector<Message> snapshot;
    std::string response;
    {
        std::lock_guard<std::mutex> lk(ch.mtx);
        if (!ch.members.count(nick)) {
            response = "ERROR: not in channel\n";
        } else {
            snapshot.assign(ch.messages.begin(), ch.messages.end());
        }
    }
    
    if (!response.empty()) {
        safe_send(client_fd, response);
        return;
    }

    std::string header = "OK " + std::to_string(snapshot.size()) + "\n";
    if (!safe_send(client_fd, header)) {
        return;
    }

    for (const auto& msg : snapshot) {
        std::string line = msg.nick + ": " + msg.text + "\n";
        if (!safe_send(client_fd, line)) {
            break;
        }
    }
}
