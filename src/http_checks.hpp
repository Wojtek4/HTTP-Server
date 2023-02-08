#ifndef HTTP_CHECKS_HPP
#define HTTP_CHECKS_HPP

#include <string>

bool valid_method(const std::string &s);
bool valid_target(const std::string &s);
bool valid_version(const std::string &s);
bool valid_option_name(const std::string &s);
bool valid_option_value(const std::string &s);

#endif /* HTTP_CHECKS_HPP */
