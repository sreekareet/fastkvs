#include "../src/kvstore.h"
#include "../src/thread_pool.h"

#include <iostream>
#include <chrono>
#include <atomic>
#include <vector>
#include <thread>

using Clock = std::chrono::high_resolution_clock; //like typedef--alias

constexpr int NUM_OPS = 200000;
constexpr int WARMUP_OPS = 10000;

/* ---------------------------------------------------------
   Utility: Print Throughput Safely
---------------------------------------------------------- */
void print_result(const std::string& label,
                  long long duration_ms,
                  int ops) {

    std::cout << label << " Time: " << duration_ms << " ms\n";

    if (duration_ms > 0) {
        double throughput = ops * 1000.0 / duration_ms;
        std::cout << label << " Throughput: "
                  << throughput << " ops/sec\n";
    }

    std::cout << "--------------------------------------\n";
}

/* ---------------------------------------------------------
   Single Thread Benchmark
---------------------------------------------------------- */
void benchmark_single_thread() {

    std::cout << "\n===== Single Thread Benchmark =====\n";

    KVStore store(NUM_OPS);

    // Warmup phase
    for (int i = 0; i < WARMUP_OPS; ++i) {
        store.put("warmup" + std::to_string(i), "value");
    }

    // PUT benchmark
    auto start_put = Clock::now();

    for (int i = 0; i < NUM_OPS; ++i) {
        store.put("key" + std::to_string(i),
                  "value" + std::to_string(i));
    }

    auto end_put = Clock::now();

    // GET benchmark
    auto start_get = Clock::now();

    for (int i = 0; i < NUM_OPS; ++i) {
        auto value = store.get("key" + std::to_string(i));
    }

    auto end_get = Clock::now();

    auto put_time =
        std::chrono::duration_cast<std::chrono::milliseconds>(end_put - start_put).count();

    auto get_time =
        std::chrono::duration_cast<std::chrono::milliseconds>(end_get - start_get).count();

    print_result("PUT", put_time, NUM_OPS);
    print_result("GET", get_time, NUM_OPS);
}

/* ---------------------------------------------------------
   Multi-Thread Benchmark (PUT only)
---------------------------------------------------------- */
void benchmark_multi_thread(int threads) {

    std::cout << "\n===== Multi-Thread Benchmark (" 
              << threads << " threads) =====\n";

    KVStore store(NUM_OPS);
    ThreadPool pool(threads);

    std::atomic<int> completed(0);

    // Warmup
    for (int i = 0; i < WARMUP_OPS; ++i) {
        store.put("warmup" + std::to_string(i), "value");
    }

    auto start = Clock::now();

    for (int i = 0; i < NUM_OPS; ++i) {
        pool.enqueue([&store, &completed, i]() {
            store.put("key" + std::to_string(i),
                      "value" + std::to_string(i));
            completed.fetch_add(1, std::memory_order_relaxed);
        });
    }

    // Wait for completion
    while (completed.load(std::memory_order_relaxed) < NUM_OPS) {
        std::this_thread::yield();
    }

    auto end = Clock::now();

    auto duration =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    print_result("MULTI-PUT", duration, NUM_OPS);
}

/* ---------------------------------------------------------
   Mixed Workload Benchmark (80% GET / 20% PUT)
---------------------------------------------------------- */
void benchmark_mixed_workload(int threads) {

    std::cout << "\n===== Mixed Workload (80% GET / 20% PUT) - "
              << threads << " threads =====\n";

    KVStore store(NUM_OPS);
    ThreadPool pool(threads);
    std::atomic<int> completed(0);

    // Pre-fill store
    for (int i = 0; i < NUM_OPS; ++i) {
        store.put("key" + std::to_string(i),
                  "value" + std::to_string(i));
    }

    auto start = Clock::now();

    for (int i = 0; i < NUM_OPS; ++i) {
        pool.enqueue([&store, &completed, i]() {

            if (i % 5 == 0) {  // 20% PUT
                store.put("key" + std::to_string(i),
                          "newvalue" + std::to_string(i));
            } else {          // 80% GET
                auto value = store.get("key" + std::to_string(i));
            }

            completed.fetch_add(1, std::memory_order_relaxed);
        });
    }

    while (completed.load(std::memory_order_relaxed) < NUM_OPS) {
        std::this_thread::yield();
    }

    auto end = Clock::now();

    auto duration =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    print_result("MIXED", duration, NUM_OPS);
}

/* ---------------------------------------------------------
   Main
---------------------------------------------------------- */
int main() {

    benchmark_single_thread();

    std::vector<int> thread_counts = {1, 2, 4, 8};

    for (int t : thread_counts) {
        benchmark_multi_thread(t);
        benchmark_mixed_workload(t);
    }

    return 0;
}

