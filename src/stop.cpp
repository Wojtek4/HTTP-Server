#include <atomic>

namespace stop {
    namespace {
        std::atomic_bool stop(false);
    }
    bool is_stopped() {
        return stop.load();
    }
    void stop_server(__attribute__((unused)) int signal) {
        stop = true;
    }
}