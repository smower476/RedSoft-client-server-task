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

struct Message {
    std::string nick;
    std::string text;
    Message(const std::string& n, const std::string& t) : nick(n), text(t) {}
};

struct Channel {
    std::deque<Message> messages;
    std::set<std::string> members;
    std::mutex mtx;
};

namespace {
    std::mutex channels_mutex;
    std::map<std::string, std::shared_ptr<Channel>> channels;
}

ClientHandler::ClientHandler(int client_fd,
                             const std::atomic<bool>& stop_flag,
                             std::mutex& client_sockets_mutex,
                             std::set<int>& client_sockets)
    : client_fd_(client_fd),
      stop_flag_(stop_flag),
      client_sockets_mutex_(client_sockets_mutex),
      client_sockets_(client_sockets) {}

void ClientHandler::operator()() {
    std::string line;
    while (!stop_flag_ && recvLine(client_fd_, line)) {
        std::string cmd = trim(line);
        if (!cmd.empty()) {
            process_command(cmd);
        }
    }
    
    close(client_fd_);
    {
        std::lock_guard<std::mutex> lock(client_sockets_mutex_);
        client_sockets_.erase(client_fd_);
    }
}

void ClientHandler::process_command(const std::string& command) {
    std::istringstream iss(command);
    std::string action, channel_name, nick;
    iss >> action >> channel_name >> nick;
    
    if (action.empty() || channel_name.empty() || nick.empty()) {
        safe_send(client_fd_, "ERROR: invalid command\n");
        return;
    }
    
    if (channel_name.size() > 24 || nick.size() > 24) {
        safe_send(client_fd_, "ERROR: channel or nick too long\n");
        return;
    }

    auto ch_ptr = GetOrCreateChannel(channel_name, action);
    if (!ch_ptr) {
        safe_send(client_fd_, "ERROR: no such channel\n");
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
        safe_send(client_fd_, "ERROR: unknown command\n");
    }
}

std::shared_ptr<Channel> ClientHandler::GetOrCreateChannel(
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
    safe_send(client_fd_, response);
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
    safe_send(client_fd_, response);
}

void ClientHandler::handleSend(Channel& ch, const std::string& nick, const std::string& message) {
    if (message.empty()) {
        safe_send(client_fd_, "ERROR: message cannot be empty\n");
        return;
    }

    std::string truncated = message;
    if (truncated.size() > 256) {
        truncated.resize(256);
    }

    std::string response;
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
    safe_send(client_fd_, response);
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
        safe_send(client_fd_, response);
        return;
    }

    std::string header = "OK " + std::to_string(snapshot.size()) + "\n";
    if (!safe_send(client_fd_, header)) {
        return;
    }

    for (const auto& msg : snapshot) {
        std::string line = msg.nick + ": " + msg.text + "\n";
        if (!safe_send(client_fd_, line)) {
            break;
        }
    }
}
