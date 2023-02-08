#ifndef STRING_UTILS_HPP
#define STRING_UTILS_HPP

#include <string>

void remove_whitespaces(std::string &s);
bool is_number(const std::string &s);
bool ends_with(const std::string &s, const char* s2);
bool starts_with(const std::string &s, const char* s2);

#endif /* STRING_UTILS_HPP */
