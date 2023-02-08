#ifndef STOP_HPP
#define STOP_HPP

#include <exception>
#include <atomic>

namespace stop {
    
    bool is_stopped();
    void stop_server(int signal);

    class StopException : std::exception {};
}

#endif /* STOP_HPP */
