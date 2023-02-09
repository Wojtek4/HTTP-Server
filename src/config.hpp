#ifndef SRC_CONFIG_HPP
#define SRC_CONFIG_HPP

#include <string>

constexpr int PORT = 50005;
const std::string WORKING_DIR = "./public_html";

#ifndef DEBUG
#define DEBUG 0
#endif

constexpr int MAX_QUEUE_LENGTH = 100000;
constexpr int WORKERS = 8;

#endif /* SRC_CONFIG_HPP */
