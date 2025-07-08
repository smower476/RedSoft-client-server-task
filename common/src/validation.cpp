#include <algorithm>
#include <string>
#include <validation.h>

std::string trim(const std::string &s) {
    auto wsfront = find_if_not(s.begin(), s.end(), [](int c){ return isspace(c); });
    auto wsback = find_if_not(s.rbegin(), s.rend(), [](int c){ return isspace(c); }).base();
    if (wsback <= wsfront) return "";
    return std::string(wsfront, wsback);
}
