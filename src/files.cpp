#include <string>
#include <unordered_map>
#include <mutex>
#include <sys/stat.h>
#include <fcntl.h>
#include "files.hpp"
#include "log.hpp"

namespace files {
    namespace {
        const std::string perm_string = "PERM";
        const std::string not_found_string = "NOT_FOUND";
        const std::string int_err_string = "INT_ERR";
        std::unordered_map<std::string, const std::string*> files;
        std::mutex m;
    }
    const std::string* const perm = &perm_string;
    const std::string* const not_found = &not_found_string;
    const std::string* const int_err = &int_err_string;
    const std::string* get_file_content(const std::string &target) {
        std::lock_guard<std::mutex> guard(m);
        auto ptr = files.find(target);
        if (ptr != files.end()) {
            auto res = ptr->second;
            return res;
        }
        int fd = open(target.c_str(), O_RDONLY);
        if (fd == -1) {
            auto res = not_found;
            if (errno == EACCES) {
                res = perm;
            }
            files[target] = res;
            return res;
        }
        struct stat stat_buff;
        fstat(fd, &stat_buff);
        size_t block_size = stat_buff.st_blksize;
        size_t blocks_to_read = (stat_buff.st_size + block_size - 1) / block_size;
        size_t required_buffer_length = block_size * blocks_to_read + 1;
        std::string *data = new std::string(required_buffer_length, '\0');
        FILE* fp = fdopen(fd, "r");
        int read_length = fread(&((*data)[0]), block_size, blocks_to_read, fp);
        log(std::to_string(read_length).c_str());
        if (read_length == -1 || ferror(fp)) {
            delete data;
            files[target] = int_err;
            return int_err;
        }
        fclose(fp);
        data->resize(stat_buff.st_size);
        files[target] = data;
        ptr = files.find(target);
        auto res = ptr->second;
        return res;
    }
    void clear() {
        std::lock_guard<std::mutex> guard(m);
        for (auto &file : files) {
            auto ptr = file.second;
            if (ptr != perm && ptr != not_found && ptr != int_err)
                delete ptr;
        }
        files.clear();
    }
}