#include <iostream>
#include "config.hpp"

void log(const char* s) {
    if (DEBUG) {
        std::cout << s << std::endl;
    }
}