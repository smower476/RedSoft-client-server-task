#include <iostream>
#include <string>
#include <unistd.h>
#include <arpa/inet.h>
#include <InputValidation.h>
#include <connection.h>
#include <commands.h>

bool parseArguments(int argc, char *argv[],std::string &server_ip, int &port, std::string &channel) {
    if (argc != 4) {
        std::cerr << "Использование: client <server_ip> <port> <channel>" << std::endl;
        return false;
    }

    server_ip = argv[1];
    port = std::stoi(argv[2]);
    channel = argv[3];

    if (channel.size() > 24) {
        std::cerr << "Имя канала слишком длинное (максимум 24 символа)" << std::endl;
        return false;
    }
    return true;
}

int main(int argc, char *argv[]) {
   std::string server_ip, channel;
    int port;

    if (!parseArguments(argc, argv, server_ip, port, channel)) return 1;

   std::string nick = inputNickname();
    int sock = connectToServer(server_ip, port);
    if (sock < 0) return 2;

    std::cout << "Подключен к серверу " << server_ip << ":" << port
         << ", начальный канал: " << channel << std::endl;

    CommandHandler handler(sock, channel, nick);
    handler.run();

    close(sock);
    return 0;
}
