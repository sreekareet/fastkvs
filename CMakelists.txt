cmake_minimum_required(VERSION 3.10)
project(FastKV)
set(CMAKE_CXX_STANDARD 17)

find_package(Threads REQUIRED)

# -------------------------------------------------------
# Shared library of all core KVStore source files
# -------------------------------------------------------
# Instead of repeating src/*.cpp in every executable,
# we compile them once into a single "kvcore" library.
# Every executable just links against it.

add_library(kvcore
    src/kvstore.cpp
    src/thread_pool.cpp
    src/lru_cache.cpp
    src/persistence.cpp
)
target_link_libraries(kvcore PUBLIC Threads::Threads)

# -------------------------------------------------------
# Executables — each just needs its own .cpp + kvcore
# -------------------------------------------------------

add_executable(fastkv src/main.cpp)
target_link_libraries(fastkv PRIVATE kvcore)
target_compile_options(fastkv PRIVATE -Wall -Wextra -Wpedantic)

add_executable(test test/lru_evict_test.cpp)
target_link_libraries(test PRIVATE kvcore)

add_executable(benchmark benchmarks/benchmark_kvstore.cpp)
target_link_libraries(benchmark PRIVATE kvcore)

add_executable(server server.cpp)
target_link_libraries(server PRIVATE kvcore)

add_executable(client client.cpp)
# client.cpp only uses sockets, no KVStore — no kvcore needed
