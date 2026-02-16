#include "../src/kvstore.h"
#include <iostream>
#include <chrono>

int main() {
    const int NUM_OPS = 100000;
    KVStore store(10000);

    auto start = std::chrono::high_resolution_clock::now();

    // PUT benchmark
    for (int i = 0; i < NUM_OPS; i++) {
        store.put("key" + std::to_string(i), "value" + std::to_string(i));
    }

    auto mid = std::chrono::high_resolution_clock::now();

    std::string value;

    // GET benchmark
    for (int i = 0; i < NUM_OPS; i++) {
        store.get("key" + std::to_string(i), value);
    }

    auto end = std::chrono::high_resolution_clock::now();

    auto put_time = std::chrono::duration_cast<std::chrono::milliseconds>(mid - start).count();
    auto get_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - mid).count();

    std::cout << "PUT Time: " << put_time << " ms\n";
    std::cout << "GET Time: " << get_time << " ms\n";

    std::cout << "PUT Throughput: " 
              << (NUM_OPS * 1000.0 / put_time) 
              << " ops/sec\n";

    std::cout << "GET Throughput: " 
              << (NUM_OPS * 1000.0 / get_time) 
              << " ops/sec\n";

    return 0;
}