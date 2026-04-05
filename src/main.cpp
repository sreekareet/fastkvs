#include "kvstore.h"
#include "thread_pool.h"
#include "persistence.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <string>

int main() {

    std::mutex cout_mutex;    
    std::cout << "FastKV starting...\n";

    // Initialize KV Store with capacity 1000
    KVStore store(1000);

    // Initialize persistence with file "data.db"
    Persistence persistence("data.db");
    persistence.load(store);

    // Thread pool with 4 threads
    ThreadPool pool(4);

    // Concurrent PUT operations
    const int num_ops = 100;
    for (int i = 0; i < num_ops; ++i) {
        pool.enqueue([&store, i]() {
            store.put("key" + std::to_string(i), "value" + std::to_string(i));
        });
    }

    // Concurrent GET operations
    for (int i = 0; i < num_ops; ++i) {
        pool.enqueue([&, i]() {
            std::string value;
            if (auto value = store.get("key" + std::to_string(i))) {
                // For demo only; logging may be used
                std::lock_guard<std::mutex> lock(cout_mutex);
                std::cout << "Got: " << *value << "\n";
            }
        });
    }

    // Allow time for threads to finish
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // Save current KV store to disk
    persistence.save(store);

    std::cout << "FastKV completed.\n";
    return 0;
}
