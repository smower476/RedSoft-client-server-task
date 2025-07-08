#include <iostream>
#include <algorithm>
#include <validation.h>
using namespace std;

string trim(const string &s) {
    auto wsfront = find_if_not(s.begin(), s.end(), [](int c){ return isspace(c); });
    auto wsback = find_if_not(s.rbegin(), s.rend(), [](int c){ return isspace(c); }).base();
    if (wsback <= wsfront) return "";
    return string(wsfront, wsback);
}

bool isValidNick(const string &nick) {
    for (char c : nick) {
        if (!isalnum(c) && c != '_') {
            return false;
        }
    }
    return true;
}

string inputNickname() {
    string nick;
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
