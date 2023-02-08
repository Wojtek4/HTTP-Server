#include <unordered_set>
#include <mutex>
#include "task.hpp"

namespace tasks_set {
    namespace {
        std::unordered_set<Task*> s;
        std::mutex m;
        bool clear_in_progress = false;
    }
    void add(Task* task) {
        std::lock_guard<std::mutex> guard(m);
        s.insert(task);
    }
    void remove(Task* task) {
        if (clear_in_progress)
            return;
        std::lock_guard<std::mutex> guard(m);
        s.erase(task);
    }
    void clear() {
        clear_in_progress = true;
        std::lock_guard<std::mutex> guard(m);
        for (auto &task : s) {
            task->close_task();
        }
        s.clear();
    }
}