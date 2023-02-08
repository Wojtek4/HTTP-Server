#include <sys/epoll.h>
#include <unistd.h>
#include "task.hpp"
#include "stop.hpp"
#include "socket.hpp"
#include "http_checks.hpp"
#include "string_utils.hpp"
#include "tasks_set.hpp"
#include "config.hpp"
#include "files.hpp"

void Task::closed_read() {
    closing_read = true;
}

void Task::closed_write() {
    closing_write = true;
}

void Task::continue_task() {

    // process all complete requests untill
    // writing or reading would block
    while(true) {

        if (state != SEND_RESPONSE && !closing_read)
            read_all();
    
        if (state == READ_METHOD) {
            size_t space_position = input_data.find(' ', first_not_parsed);
            if (space_position != std::string::npos) {
                method = input_data.substr(first_not_parsed, space_position - first_not_parsed);
                first_not_parsed = space_position + 1;
                state = READ_TARGET;
                if (!valid_method(method)) {
                    send_error_and_close(400);
                    return;
                }
            }
        }
        
        if (state == READ_TARGET) {
            size_t space_position = input_data.find(' ', first_not_parsed);
            if (space_position != std::string::npos) {
                target = input_data.substr(first_not_parsed, space_position - first_not_parsed);
                first_not_parsed = space_position + 1;
                state = READ_VERSION;
                if (!valid_target(version)) {
                    send_error_and_close(400);
                    return;
                }
            }
        }

        if (state == READ_VERSION) {
            size_t crlf_position = input_data.find("\r\n", first_not_parsed);
            if (crlf_position != std::string::npos) {
                version = input_data.substr(first_not_parsed, crlf_position - first_not_parsed);
                first_not_parsed = crlf_position + 2;
                state = READ_OPTIONS;
                if (!valid_version(version)) {
                    send_error_and_close(400);
                    return;
                }
            }
        }

        if (state == READ_OPTIONS) {
            while (true) {
                size_t crlf_position = input_data.find("\r\n", first_not_parsed);
                if (crlf_position == first_not_parsed) {
                    first_not_parsed += 2;
                    determine_body_length();
                    body.clear();
                    state = READ_BODY;
                    break;
                }
                if (crlf_position != std::string::npos) {
                    size_t sep_position = input_data.find(':', first_not_parsed);
                    if (sep_position != std::string::npos && sep_position < crlf_position) {
                        std::string name = input_data.substr(first_not_parsed, sep_position - first_not_parsed);
                        std::string value = input_data.substr(sep_position + 1, crlf_position - sep_position - 1);
                        if (valid_option_name(name) && valid_option_value(value)) {
                            remove_whitespaces(value);
                            if (options.find(name) == options.end())
                                options[name] = value;
                            else {
                                options[name] += ',';
                                options[name] += value;
                            }
                            first_not_parsed = crlf_position + 2;
                        }
                        else {
                            send_error_and_close(400);
                            return;
                        }
                    }
                    else {
                        send_error_and_close(400);
                        return;
                    }
                }
                else {
                    break;
                }
            }
        }

        if (state == READ_BODY) {
            if (body_length == -2) {
                send_error_and_close(400);
                return;
            }
            if (body_length == -1) {
                // Assume all data has arrived.
                body += input_data.substr(first_not_parsed);
                input_data.clear();
                first_not_parsed = 0;
                process_request();
                state = SEND_RESPONSE;
            }
            else {
                size_t added_length = std::min(body_length - body.size(), input_data.size() - first_not_parsed);
                body += input_data.substr(first_not_parsed, added_length);
                first_not_parsed += added_length;
                if (body.size() == (size_t)body_length) {
                    process_request();
                    state = SEND_RESPONSE;
                }
            }
        }

        if (state == SEND_RESPONSE && !closing_write) {
            send_all();
            if (first_not_sent == response.size()) {
                request_cleanup();
                continue;
            }
        }

        break;
    }

    if (closing_write) {
        close_task();
        return;
    }
    if (closing_read && state != SEND_RESPONSE) {
        if (state == READ_METHOD && first_not_parsed == input_data.size()) {
            close_task();
        }
        else {
            send_error_and_close(400);
        }
        return;
    }

    // rearm socket in epoll
    epoll_event event;
    event.data.ptr = this;
    event.events = EPOLLET | EPOLLONESHOT | EPOLLPRI;
    if (state == SEND_RESPONSE)
        event.events |= EPOLLOUT;
    else
        event.events |= EPOLLIN;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, socket, &event) == -1) {
        perror("epoll_ctl_mod: listen_socket");
        exit(EXIT_FAILURE);
    }
}

void Task::read_all() {
    constexpr int read_length = 2048;
    std::string buff(read_length + 1, '\0');
    while(true) {
        errno = 0;
        ssize_t read_size = read(socket, &(buff[0]), read_length);
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return;
        }
        if (read_size == -1 && errno == EINTR) {
            continue;
        }
        if (read_size <= 0 ) {
            closed_read();
            return;
        }
        input_data.append(buff, 0, read_size);
    }
}

void Task::send_all() {
    while(first_not_sent < response.size()) {
        errno = 0;
        ssize_t sent_size = write(socket, &(response[first_not_sent]), response.size() - first_not_sent);
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return;
        }
        if (sent_size <= 0) {
            if (errno == EINTR) {
                continue;
            }
            closed_write();
            return;
        }
        first_not_sent += sent_size;
    }
}

void Task::process_request() {
    if (method == "GET" || method == "HEAD") {
        preprocess_target();
        const std::string *file_content = files::get_file_content(target);
        if (file_content == files::int_err) {
            make_error_response(500);
            return;
        }
        if (file_content == files::perm) {
            make_error_response(403);
            return;
        }
        if (file_content == files::not_found) {
            make_error_response(404);
            return;
        }
        response = "HTTP/1.1 200 \r\n";
        response += "Content-Length: ";
        response += std::to_string(file_content->size());
        response += "\r\n\r\n";
        if (method == "GET")
            response += *file_content;
        first_not_sent = 0;
    }
    else {
        make_error_response(501);
    }
}
void Task::close_task() {
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, socket, NULL);
    close(socket);
    tasks_set::remove(this);
    delete this;
}
void Task::determine_body_length() {
    body_length = 0;
    auto ptr_length = options.find("Content-Length");
    auto ptr_encoding = options.find("Transfer-Encoding");
    if (ptr_encoding != options.end()) {
        if (ends_with(ptr_encoding->second, "chunked")) {
            body_length = -1;
        }
        else {
            body_length = -2;
        }
    }
    else if (ptr_length != options.end()) {
        if (is_number(ptr_length->second)) {
            body_length = std::stoi(ptr_length->second);
        }
        else {
            body_length = -2;
        }
    }
}

void Task::preprocess_target() {
    int start_search_from = 0;
    if (starts_with(target, "http://")) {
        start_search_from = 7;
    }
    else if (starts_with(target, "https://")) {
        start_search_from = 8;
    }
    size_t start = target.find('/', start_search_from);
    if (start == std::string::npos) {
        return;
    }
    size_t end = target.find('?', start);
    if (end == std::string::npos)
        target = target.substr(start);
    else
        target = target.substr(start, end - start);
    target = WORKING_DIR + target;
}

void Task::request_cleanup() {
    method.clear();
    target.clear();
    version.clear();
    options.clear();
    body.clear();
    body_length = 0;
    state = READ_METHOD;

    if (first_not_parsed > input_data.size() / 2) {
        input_data = input_data.substr(first_not_parsed);
        first_not_parsed = 0;
    }

    response.clear();
    first_not_sent = 0;
}
void Task::make_error_response(int code) {
    response = "HTTP/1.1 " + std::to_string(code) + " \r\n"
        + "Content-Length:0\r\n\r\n";
}
void Task::send_error_and_close(int code) {
    make_error_response(code);
    send_all();
    close_task();
}
