#ifndef INC_TASK_HPP
#define INC_TASK_HPP

#include <map>
#include <string>

enum TaskState {
    READ_METHOD,
    READ_TARGET,
    READ_VERSION,
    READ_OPTIONS,
    READ_BODY,
    SEND_RESPONSE
};

struct Task {
    int socket, epoll_fd;

    std::string method, target, version;
    std::map<std::string, std::string> options;
    std::string body;
    ssize_t body_length;

    std::string input_data;
    size_t first_not_parsed;

    std::string response;
    size_t first_not_sent;

    TaskState state;

    bool closing_read, closing_write;
    void closed_read();
    void closed_write();

    Task(int s, int ep_fd) {
        socket = s;
        epoll_fd = ep_fd;
        state = READ_METHOD;
    }
    
    void read_all();
    void send_all();
    void process_request();
    void preprocess_target();
    void close_task();
    void determine_body_length();
    void request_cleanup();
    void send_error_and_close(int code);
    void make_error_response(int code);

    void continue_task();
};

#endif /* INC_TASK_HPP */
