#include "../src/kvstore.h"
#include "../src/thread_pool.h"
#include <iostream>
#include <atomic>
#include <chrono>

int main() {

    const int NUM_TASKS = 100000;
    KVStore store(200000);
    ThreadPool pool(4);

    std::atomic<int> completed(0); //to avoid race condition

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < NUM_TASKS; i++) {
        pool.enqueue([&store, &completed, i]() {
            store.put("key" + std::to_string(i),
                      "value" + std::to_string(i));
            completed++;
        });
    }

    // Wait until all tasks complete
    while (completed.load() < NUM_TASKS) {
        std::this_thread::yield();
    }

    auto end = std::chrono::high_resolution_clock::now();

    auto duration =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    std::cout << "Total operations: " << NUM_TASKS << "\n";
    std::cout << "Time: " << duration << " ms\n";
    std::cout << "Throughput: "
              << (NUM_TASKS * 1000.0 / duration)
              << " ops/sec\n";

    return 0;
}
