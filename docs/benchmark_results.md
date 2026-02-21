## FastKVS Benchmark Results

Date: 2026-02-20

KVStore Capacity: 200,000 keys

# Single Thread Benchmark

| Operation | Time (ms) | Throughput (ops/sec) |
| --------- | --------- | -------------------- |
| PUT       | 314       | 636,943              |
| GET       | 146       | 1,369,860            |

Notes:

- PUT throughput is lower than GET due to write and LRU updates.
- GET throughput is higher because reads are relatively cheaper, even though LRU reordering occurs.

# Multi-Thread PUT Benchmark

| Threads | Time (ms) | Throughput (ops/sec) |
| ------- | --------- | -------------------- |
| 1       | 523       | 382,409              |
| 2       | 1178      | 169,779              |
| 4       | 2118      | 94,428.7             |
| 8       | 2737      | 73,072.7             |

Notes:

- Throughput decreases at higher threads due to mutex contention in KVStore.
- LRU eviction overhead adds slightly to each PUT.

# Mixed Workload (80% GET / 20% PUT)

| Threads | Time (ms) | Throughput (ops/sec) |
| ------- | --------- | -------------------- |
| 1       | 595       | 336,134              |
| 2       | 1783      | 112,170              |
| 4       | 1494      | 133,869              |
| 8       | 2501      | 79,968               |

Notes:

- Mixed workloads reflect realistic usage scenarios.
- GET operations dominate, which helps throughput stay higher than pure PUT with multiple threads.
- Shows impact of thread contention vs. read/write mix.

# Observations & Analysis

- GET operations are faster than PUT due to lower write and LRU manipulation overhead.
- Increasing threads beyond 4 for multi-thread PUT reduces throughput due to global mutex contention.
- Mixed workloads (80% GET / 20% PUT) maintain relatively stable throughput, making FastKVS efficient under read-heavy loads.
- The current design can benefit from shared_mutex for read-only GETs if LRU reordering is made optional or separated.
- Future improvements could include finer-grained locking or lock-free LRU for better scaling.

# Additional Information

- Benchmarks measure ops/sec for PUT, GET, and mixed workloads.
- Timings are measured in milliseconds using std::chrono.

Throughput calculation:

Throughput (ops/sec) = (Total Operations × 1000) / Time (ms)
