FastKVS:

FastKVS is a simple multithreaded in-memory key-value store written in C++17.

This project demonstrates LRU cache eviction, custom thread pool implementation, and basic persistence.

Features:

> In-memory key-value storage

> LRU (Least Recently Used) eviction policy

> Custom ThreadPool using std::thread

> Mutex and condition variable for synchronization

> Basic file persistence

> Single thread and multi thread benchmarks

> Built using CMake

Project Structure:

fastkvs/
│
├── src/        # Core source files
├── test/       # Benchmark and test files
├── CMakeLists.txt
├── README.md
└── .gitignore

Build Instructions:

Step 1: Create build folder
mkdir build
cd build

Step 2: Run CMake in Release mode
cmake -DCMAKE_BUILD_TYPE=Release ..

Step 3: Compile
make

Run Benchmarks:

After building:

./benchmark_single
./benchmark_multi

Technologies Used:

C++17

STL (unordered_map, list, thread, mutex, condition_variable)

CMake

Git

Author:

Sreekaree

GitHub: https://github.com/sreekareet/fastkvs
