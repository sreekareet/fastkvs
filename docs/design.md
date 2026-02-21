## KVStore Detailed Design

# 1. Design Goals

- Thread safe operations
- O(1) average time complexity for put/get/remove
- LRU eviction policy
- Clean modular architecture
- Benchmarkable and testable

------------------------------------------------------------

## 2. Core Data Structures

# 2.1 Hash Map

Type:
    std::unordered_map<std::string, std::string>

Purpose:
    Fast O(1) key lookup

# 2.2 Doubly Linked List (LRU)

Purpose:
    Track usage order

Head:
    Most recently used

Tail:
    Least recently used (eviction candidate)

Operations:
- Move node to head (on get)
- Remove tail (on eviction)
- Insert at head (on put)

Time Complexity:
    O(1)

------------------------------------------------------------

 ## 3. Thread Safety Design

 # 3.1 Mutex Strategy

Type:
    std::shared_mutex kv_mutex

Reason:
    Allow future scalability improvements

Current Usage:
    std::unique_lock used for:
        - put()
        - get()
        - remove()

Why get() uses unique_lock:
    Because it modifies LRU ordering.

------------------------------------------------------------

 # 4. Operation Complexity

| Operation | Time Complexity |
|-----------|-----------------|
| put       | O(1) average    |
| get       | O(1) average    |
| remove    | O(1) average    |
| eviction  | O(1)            |

------------------------------------------------------------

# 5. Persistence Design

Persistence class:
 save(const KVStore&)
 load(KVStore&)

Storage Format:
    key value\n

Limitation:
- No crash recovery
- No WAL (Write-Ahead Logging)

------------------------------------------------------------

# 6. Testing Strategy

Unit Tests:
- put/get correctness
- overwrite behavior
- removal behavior
- LRU eviction correctness

Location:
    test/lru_evict_test.cpp

------------------------------------------------------------

# 7. Benchmarking Strategy

Location:
    benchmarks/benchmark_kvstore.cpp

Measured Metrics:
- Throughput (ops/sec)
- Latency
- Multi thread scalability

Results documented in:
    docs/benchmark_results.md

------------------------------------------------------------

# 8. Known Limitations

- Single global lock per KVStore instance
- Not optimized for high contention workloads
- No distributed support
- No persistence durability guarantees

------------------------------------------------------------

# 9. Future Enhancements

- Concurrent LRU
- Background eviction thread
- Write head Logging

------------------------------------------------------------
