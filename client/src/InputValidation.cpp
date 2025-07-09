#include <iostream>
#include <InputValidation.h>
#include <validation.h>

bool is_valid_nick(const std::string &nick) {
    for (char c : nick) {
        if (!isalnum(c) && c != '_') {
            return false;
        }
    }
    return true;
}

std::string input_nickname() {
    std::string nick;
    while (true) {
        std::cout << "Введите ваш ник (max 24 символа): ";
        getline(std::cin, nick);
        nick = trim(nick);

        if (nick.empty() || nick.size() > 24 || !is_valid_nick(nick)) {
            std::cerr << "Неверный ник, попробуйте снова." << std::endl;
        } else {
            return nick;
        }
    }
}
