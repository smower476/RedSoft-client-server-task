#include <iostream>
#include <InputValidation.h>
#include <validation.h>

bool isValidNick(const std::string &nick) {
    for (char c : nick) {
        if (!isalnum(c) && c != '_') {
            return false;
        }
    }
    return true;
}

std::string inputNickname() {
    std::string nick;
    while (true) {
        cout << "Введите ваш ник (max 24 символа): ";
        getline(cin, nick);
        nick = trim(nick);

        if (nick.empty() || nick.size() > 24 || !isValidNick(nick)) {
            cerr << "Неверный ник, попробуйте снова." << endl;
        } else {
            return nick;
        }
    }
}
