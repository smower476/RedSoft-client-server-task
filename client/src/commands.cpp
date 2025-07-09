#include <connection.h>
#include <commands.h>
#include <InputValidation.h>
#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <poll.h>
#include <sstream>
#include <validation.h>
#include <network_utils.h>

CommandHandler::CommandHandler(int sock, const std::string& channel, const std::string& nick)
    : sock(sock), channel(channel), nick(nick) {}

bool CommandHandler::handleSend(std::istringstream& iss) {
    std::string msg;
    getline(iss, msg);
    msg = trim(msg);
    if (msg.empty()) {
        std::cout << "Использование: send <сообщение>" << std::endl;
        return true;
    }

    std::string request = "send " + channel + " " + nick + " " + msg + "\n";
    if (!safe_send(sock, request)) {
        std::cout << "Не удалось отправить сообщение." << std::endl;
        return false;
    }

    std::string response;
    if (!recvLine(sock, response)) {
        std::cout << "Отключено от сервера после send." << std::endl;
        return false;
    }
    if (response.rfind("OK", 0) != 0) {
        std::cout << "Ошибка сервера: " << response << std::endl;
    }
    return true;
}

bool CommandHandler::handleRead() {
    std::string request = "read " + channel + " " + nick + "\n";
    if (!safe_send(sock, request)) {
        std::cout << "Не удалось отправить запрос чтения." << std::endl;
        return false;
    }

    std::string response;
    if (!recvLine(sock, response)) {
        std::cout << "Отключено от сервера." << std::endl;
        return false;
    }

    if (response.rfind("OK", 0) == 0) {
        std::istringstream rs(response);
        std::string ok;
        int count;
        rs >> ok >> count;
        std::cout << "Последние " << count << " сообщений в '" << channel << "':\n";
        for (int i = 0; i < count; ++i) {
            if (!recvLine(sock, response)) break;
            std::cout << response << std::endl;
        }
    } else {
        std::cout << "Ошибка: " << response << std::endl;
    }
    return true;
}

bool CommandHandler::handleJoin(std::istringstream& iss) {
    std::string new_channel;
    iss >> new_channel;
    if (new_channel.empty()) {
        std::cout << "Использование: join <канал>" << std::endl;
        return true;
    }
    if (new_channel.size() > 24) {
        std::cout << "Имя канала слишком длинное (максимум 24 символа)" << std::endl;
        return true;
    }

    std::string request = "join " + new_channel + " " + nick + "\n";
    if (!safe_send(sock, request)) {
        std::cout << "Не удалось отправить запрос присоединения." << std::endl;
        return false;
    }

    std::string response;
    if (!recvLine(sock, response)) {
        std::cout << "Отключено от сервера." << std::endl;
        return false;
    }

    if (response.rfind("OK", 0) == 0) {
        channel = new_channel;
        std::cout << "Присоединились к каналу: " << channel << std::endl;
    } else {
        std::cout << "Ошибка присоединения: " << response << std::endl;
    }
    return true;
}

bool CommandHandler::handleExit() {
    std::string request = "exit " + channel + " " + nick + "\n";
    if (!safe_send(sock, request)) {
        std::cout << "Не удалось отправить запрос выхода." << std::endl;
        return false;
    }

    std::string response;
    if (!recvLine(sock, response)) {
        std::cout << "Отключено от сервера." << std::endl;
        return false;
    }

    if (response.rfind("OK", 0) == 0) {
        std::cout << "Вышли из канала: " << channel << std::endl;
    } else {
        std::cout << "Ошибка выхода: " << response << std::endl;
    }
    return true;
}

void CommandHandler::run() {
    std::string line;
    while (true) {
        std::cout << "> ";
        if (!std::getline(std::cin, line)) break;
        line = trim(line);
        if (line.empty()) continue;
        if (line == "quit") break;

        std::istringstream iss(line);
        std::string cmd;
        iss >> cmd;

        bool status = true;
        if (cmd == "send") {
            status = handleSend(iss);
        } else if (cmd == "read") {
            status = handleRead();
        } else if (cmd == "join") {
            status = handleJoin(iss);
        } else if (cmd == "exit") {
            status = handleExit();
        } else {
            std::cout << "Неизвестная команда. Доступные: send, read, join, exit, quit.\n";
        }

        if (!status) {
            std::cout << "Соединение прервано.\n";
            break;
        }
    }
}
