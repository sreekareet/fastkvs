FastKVS

A multithreaded in-memory key-value store implemented in C++17.

Features:

LRU eviction policy
ThreadPool-based concurrency
Persistent storage
CMake build system
Benchmark tests (single-thread & multi-thread)


Build:
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make

Run Benchmarks:
./benchmark_single
./benchmark_multi
