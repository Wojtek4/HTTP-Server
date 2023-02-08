#include <csignal>
#include <sys/epoll.h>
#include "stop.hpp"
#include "threadpool.hpp"
#include "socket.hpp"
#include "task.hpp"
#include "thread_main.hpp"
#include "tasks_set.hpp"
#include "files.hpp"
#include "log.hpp"
#include "config.hpp"

/**
 * @brief Run HTTP server.
 * 
 * @param argc Comand line arguments count
 * @param argv Command line arguments
 * @return int Exit code
 */
int main() {

    signal(SIGINT, stop::stop_server);
    int epoll_fd = epoll_create1(0);
    log("epoll created");

    int listen_socket = get_listen_socket();
    epoll_event event;
    event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
    Task *task = new Task(listen_socket, epoll_fd);
    tasks_set::add(task);
    event.data.ptr = task;

    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_socket, &event);
    log("listen_socket added to epoll");

    constexpr int N = WORKERS;
    ThreadPool<N-1> thread_pool(epoll_fd, listen_socket);
    log("threadpool created");

    while(!stop::is_stopped()) {
        try {
            process_waiting_tasks(epoll_fd, listen_socket);
        } catch (stop::StopException const&) {
            break;
        }
    }

    thread_pool.stop();
    tasks_set::clear();
    files::clear();
    
    return 0;
}