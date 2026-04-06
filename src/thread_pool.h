#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>

class ThreadPool {
public:
    ThreadPool(size_t num_threads);
    ~ThreadPool();

    void enqueue(std::function<void()> task);

private:
    void worker();

    std::vector<std::thread> vworkers;
    std::queue<std::function<void()>> qtasks;

    std::mutex w_mutex;
    std::condition_variable condition;
    std::atomic<bool> stopflag;
};

#endif
