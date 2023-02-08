#include <string>
#include <cstring>

void remove_whitespaces(std::string &s) {
    if (s.empty())
        return;
    size_t left_ptr = (s[0] == ' ' ? 1 : 0);
    size_t right_ptr = (s.back() == ' ' ? s.size() - 1 : s.size());
    if (right_ptr <= left_ptr)
        s.clear();
    else
        s = s.substr(left_ptr, right_ptr - left_ptr);
}

bool is_number(const std::string& s) {
    auto it = s.begin();
    while (it != s.end() && std::isdigit(*it))
        ++it;
    return !s.empty() && it == s.end();
}

bool ends_with(const std::string &s, const char* s2) {
    size_t l2 = strlen(s2);
    if (s.size() < l2)
        return false;
    for (size_t i = 0; i < l2; i++) {
        if (s[s.size() - l2 + i] != s2[i])
            return false;
    }
    return true;
}

bool starts_with(const std::string &s, const char* s2) {
    size_t l2 = strlen(s2);
    if (s.size() < l2)
        return false;
    return !strncmp(&(s[0]), s2, l2);
}
