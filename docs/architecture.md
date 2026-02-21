# KVStore Architecture

1. Overview

The KVStore is a thread-safe in-memory key-value store with LRU eviction support.
It is designed for high performance concurrent access with controlled synchronization.

The architecture separates responsibilities into:

 KVStore (public API + thread safety)
 LRU Cache logic (eviction ordering)
 Persistence (optional disk storage)
 Benchmarking & Testing modules


------------------------------------------------------------

2. High-Level Component Diagram

                 +-----------------------+
                 |     Application       |
                 +-----------------------+
                             |
                             v
                 +-----------------------+
                 |        KVStore        |
                 |  (Thread-Safe Layer)  |
                 +-----------------------+
                             |
                             v
                 +-----------------------+
                 |   Hash Map + LRU List |
                 +-----------------------+
                             |
                             v
                 +-----------------------+
                 |     Persistence       |
                 +-----------------------+


------------------------------------------------------------

3. Internal Structure

KVStore Internals:

 std::unordered_map<std::string, std::string>
 std::shared_mutex kv_mutex

Each KVStore instance maintains its own mutex.

Lock Scope:
 All public operations acquire lock before modifying state.
 get() uses unique lock because LRU reorders entries.

------------------------------------------------------------

4. Concurrency Model

Current Strategy:
 Single shared_mutex per KVStore object
 unique_lock used for put(), get(), remove()
 get() modifies LRU order, so shared_lock is not used

Thread Access Flow:

Thread 1 ----\
Thread 2 ----- > [ kv_mutex ] --> internal structures
Thread 3 ----/

Only one modifying operation allowed at a time.

------------------------------------------------------------

5. Data Flow Example (GET)

    1. Acquire unique_lock
    2. Lookup key in hash map
    3. If found:
      - Move node to front of LRU list
      - Return value
    4. Release lock

------------------------------------------------------------

6. Eviction Flow (PUT when capacity reached)

    1. Acquire unique_lock
    2. Check capacity
    3. Remove least recently used node (tail)
    4. Insert new key at head
    5. Release lock

------------------------------------------------------------

7. Current Limitations:

 Single lock limits scalability under heavy contention
 LRU modification prevents shared read locks

------------------------------------------------------------

8. Future Improvements:
 Sharded KVStore (multiple buckets + locks)
 Read write split for pure reads
