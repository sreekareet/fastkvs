# FASTKVS

## High Performance Thread-Safe Key-Value Store (C++)

A production-style **in-memory key-value store** implemented in C++17 with:

- Thread safety using `std::shared_mutex`
- LRU eviction policy
- File persistence
- Thread pool execution
- Benchmarking & scaling analysis
- Unit testing support
- Clean CMake build system

---

## Features

- O(1) average PUT/GET
- LRU eviction
- Concurrent multi-threaded access
- Persistence to disk
- Benchmarking (single-thread & multi-thread)
- Performance scaling analysis

---

## Architecture
```
        +-------------------+
        |      Client       |
        +---------+---------+
                  |
                  v
        +-------------------+
        |     KVStore       |
        |-------------------|
        | - unordered_map   |
        | - LRU list        |
        | - shared_mutex    |
        +---------+---------+
                  |
  +---------------+---------------+
  |                               |
  v                               v
+-------------+               +----------------+
| LRU Manager |               |  Persistence   |
| (Eviction)  |               |  (File I/O)    |
+-------------+               +----------------+
|
v
+-------------------+
|    Thread Pool    |
+-------------------+

```

**Notes:**

- `KVStore` is the main interface; all client operations (`put`, `get`, `remove`) go through it.
- `LRUCache` tracks usage to evict least recently used items when capacity is exceeded.
- `ThreadPool` manages multiple worker threads to avoid creating/destroying threads per request.
- `Persistence` allows saving and loading `KVStore` to a file.

## Concurrency Design

- `std::shared_mutex` used for synchronization
- `put()` → exclusive lock
- `get()` → exclusive lock (due to LRU reordering)
- `ThreadPool` handles concurrent task execution

---

## Build Instructions (CMake)

```bash
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
````

This compiles all source files with `-O2` optimization.

**Using g++ directly:**

```bash
g++ -std=c++17 src/*.cpp -pthread -O2 -o fastkvs
```

---

## Run KVStore Example

```bash
./fastkvs
```

---

## Run Tests

```bash
./test
```

---

## Run Benchmark

```bash
./benchmark
```

---

## Benchmark Results

Full benchmark results are available in [docs/benchmark_results.md](docs/benchmark_results.md)

**Example output (single-thread & multi-thread):**

| Scenario                               | Threads | Time (ms) | Throughput (ops/sec) |
| -------------------------------------- | ------- | --------- | -------------------- |
| Single-thread PUT                      | 1       | 314       | 636,943              |
| Single-thread GET                      | 1       | 146       | 1,369,860            |
| Multi-thread PUT                       | 4       | 2,118     | 94,428.7             |
| Multi-thread Mixed (80% GET / 20% PUT) | 4       | 1,494     | 133,869              |

---

## Unit Testing

Covers:

* `put()`
* `get()`
* `remove()`
* Eviction behavior (LRU)

---

## Future Improvements

* Lock striping to reduce contention
* True shared reads without LRU modification
* Background eviction thread
* Metrics instrumentation
* Latency percentile tracking
* Sharded architecture

---

## Project Structure

```
fastkvs/
│
├── README.md
├── CMakeLists.txt
├── docs/
│   ├── architecture.md
│   ├── design.md
│   └── benchmark_results.md
├── src/
│   ├── main.cpp
│   ├── kvstore.h
│   ├── kvstore.cpp
│   ├── lru_cache.h
│   ├── lru_cache.cpp
│   ├── thread_pool.h
│   ├── thread_pool.cpp
│   ├── persistence.h
│   └── persistence.cpp
├── benchmarks/
│   └── benchmark_kvstore.cpp
└── test/
    └── lru_evict_test.cpp
```

---

## Limitations

* Global `shared_mutex` limits high-core scalability
* `get()` requires exclusive lock due to LRU update
* Persistence is synchronous

---

## Author

**Sreekaree**
