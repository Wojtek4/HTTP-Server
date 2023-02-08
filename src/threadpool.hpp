#ifndef THREADPOOL_HPP
#define THREADPOOL_HPP

#include "task.hpp"
#include "stop.hpp"
#include "thread_main.hpp"
#include "log.hpp"
#include <thread>

template<int N>
class ThreadPool {
private:
    std::thread threads[N];
public:
    ThreadPool(int epoll_fd, int listen_socket) {
        for (int i = 0; i < N; i++) {
            threads[i] = std::thread([=]{
                while(!stop::is_stopped()) {
                    log("loop");
                    try {
                        process_waiting_tasks(epoll_fd, listen_socket);
                    } catch (stop::StopException const&) {
                        return;
                    }
                }
                log("Thread from pool ends");
            });
        }
        log("threadpool constructor ends");
    }
    void stop() {
        for (int i = 0; i < N; i++) {
            threads[i].join();
        }
    }
};

#endif /* THREADPOOL_HPP */
