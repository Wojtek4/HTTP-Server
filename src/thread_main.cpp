#include <sys/epoll.h>
#include <sys/socket.h>
#include "task.hpp"
#include "stop.hpp"
#include "socket.hpp"
#include "log.hpp"

void process_waiting_tasks(int epoll_fd, int listen_socket) {
    constexpr int events_number = 1000;
    constexpr int timeout_ms = 100;
    epoll_event events[events_number];
    if (stop::is_stopped())
        throw stop::StopException();
    int n = epoll_wait(epoll_fd, events, events_number, timeout_ms);
    log("after epoll wait");
    if (n == 0) {
        log("n = 0!");
        if (stop::is_stopped()) {
            log("Throwing exception");
            throw stop::StopException();
        }
    }
    else if (n == -1) {
        log("n = 0!");
        if (errno != EINTR) {
            log("Exiting");
            exit(1);
        }
        errno = 0;
    }
    else for (int i = 0; i < n; i++) {
        Task* task = (Task*)events[i].data.ptr;

        if (task->socket == listen_socket) {

            // fetch all awaiting connections
            while(true) {
                int new_socket = accept(listen_socket, NULL, NULL);
                if (new_socket == -1) {
                    if (errno == EAGAIN || errno == EWOULDBLOCK)
                        break;
                    perror("accept: listen_socket");
                    exit(EXIT_FAILURE);
                }

                setnonblocking(new_socket);
                events[i].events = EPOLLIN | EPOLLET | EPOLLONESHOT | EPOLLPRI;
                events[i].data.ptr = new Task(new_socket, epoll_fd);
                if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, new_socket, events + i) == -1) {
                    perror("epoll_ctl: new_socket");
                    exit(EXIT_FAILURE);
                }
            }

            // rearm socket
            events[i].events = EPOLLIN | EPOLLET | EPOLLONESHOT;
            events[i].data.ptr = task;
            if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, listen_socket, events + i) == -1) {
                perror("epoll_ctl_mod: listen_socket");
                exit(EXIT_FAILURE);
            }
        }

        else {
            if ((events[i].events & (EPOLLPRI | EPOLLERR | EPOLLHUP))) {
                task->closed_read();
                task->closed_write();
            }
            task->continue_task();
        }
    }
}