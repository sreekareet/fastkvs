#include "thread_pool.h"
#include <stdexcept>

ThreadPool::ThreadPool(size_t num_threads)
    : stopflag(false) {
    for (size_t i = 0; i < num_threads; ++i) {
        vworkers.emplace_back(&ThreadPool::worker, this);
    }
}

void ThreadPool::worker() {
    while (true) {
        std::function<void()> task;

        {
            std::unique_lock<std::mutex> lock(w_mutex);
            condition.wait(lock, [this] {
                return stopflag || !qtasks.empty();
            });

            if (stopflag && qtasks.empty()) {
                return;
            }

            task = std::move(qtasks.front());
            qtasks.pop();
        }

        task();
    }
}

void ThreadPool::enqueue(std::function<void()> task) {
{
    if (stopflag) 
        throw std::runtime_error("enqueue on stopped ThreadPool");
    std::unique_lock<std::mutex> lock(w_mutex);
    qtasks.push(std::move(task));
    }

    condition.notify_one();
}

ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(w_mutex);
        stopflag = true;
    }

    condition.notify_all();

    for (auto& thread : vworkers) {
        thread.join();
    }
}
