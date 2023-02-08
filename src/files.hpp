#ifndef FILES_HPP
#define FILES_HPP

#include <string>

namespace files {
    extern const std::string* const perm;
    extern const std::string* const not_found;
    extern const std::string* const int_err;
    const std::string* get_file_content(const std::string &target);
    void clear();
}

#endif /* FILES_HPP */
