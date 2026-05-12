# FastKVS

A high-performance, thread-safe, networked key-value store written in C++17.  
Clients connect over TCP and issue `SET`, `GET`, `DEL`, and `STATS` commands against an in-memory store with LRU eviction and file persistence.

---

## Architecture

```
  Client (TCP)         Client (TCP)         Client (TCP)
       |                    |                    |
       +--------------------+--------------------+
                            |
                     [ TCP Server ]
                     port 7379
                     one thread per client
                     atomic metrics counters
                            |
                     [ KVStore ]
                     std::shared_mutex
                     unordered_map + LRU list
                      /              \
               [ LRU Cache ]    [ Persistence ]
               O(1) eviction    snapshot to disk
                      \
               [ Thread Pool ]
               worker threads
```

---

## Features

- **TCP socket server** — clients connect over the network, not just in-process
- **Concurrent client handling** — each client gets its own thread
- **Thread-safe KVStore** — `std::shared_mutex` protects all operations
- **O(1) GET/SET/DEL** — backed by `std::unordered_map`
- **LRU eviction** — bounded memory, least recently used keys evicted first
- **Real-time metrics** — hit rate, miss rate, command counts via `STATS` command
- **File persistence** — data saved to disk on shutdown, loaded on startup
- **Graceful shutdown** — handles `SIGINT`/`SIGTERM`, no data loss on Ctrl+C or kill

---

## Commands

| Command | Example | Response |
|---|---|---|
| `SET key value` | `SET name sreekar` | `OK` |
| `GET key` | `GET name` | `sreekar` or `NULL` |
| `DEL key` | `DEL name` | `OK` |
| `STATS` | `STATS` | hit rate, miss rate, command counts |

### Example STATS output
```
total_commands : 5
total_sets     : 2
total_deletes  : 1
get_hits       : 1
get_misses     : 1
hit_rate       : 50%
```

---

## Build

Requires: `g++` with C++17, CMake 3.10+, POSIX system (Linux / Cygwin)

```bash
mkdir build
cd build
cmake ..
make
```

This builds five binaries: `server`, `client`, `fastkv`, `test`, `benchmark`

---

## Run

**Start the server:**
```bash
./build/server
```

**Connect a client:**
```bash
./build/client
```

**Shut down cleanly** (data is saved to `fastkvs_data.txt`):
```
Ctrl+C
```

On next startup, data is automatically reloaded from disk.

---

## Benchmarks

| Scenario | Threads | Throughput |
|---|---|---|
| Single-thread GET | 1 | 1,369,860 ops/sec |
| Single-thread PUT | 1 | 636,943 ops/sec |
| Multi-thread PUT | 4 | 94,428 ops/sec |
| Multi-thread Mixed (80% GET / 20% PUT) | 4 | 133,869 ops/sec |

---

## Project Structure

```
fastkvs/
├── server.cpp              # TCP server — accepts clients, parses commands, tracks metrics
├── client.cpp              # TCP client — connects and sends commands
├── src/
│   ├── kvstore.h/cpp       # Core key-value store
│   ├── lru_cache.h/cpp     # LRU eviction logic
│   ├── thread_pool.h/cpp   # Worker thread pool
│   ├── persistence.h/cpp   # Save/load to disk
│   └── main.cpp            # Standalone in-process demo
├── benchmarks/
│   └── benchmark_kvstore.cpp
├── test/
│   └── lru_evict_test.cpp
├── docs/
│   └── benchmark_results.md
└── CMakeLists.txt
```

---

## Concurrency Design

- `std::shared_mutex` — single coarse-grained lock per KVStore instance
- `put()` and `remove()` → exclusive lock
- `get()` → exclusive lock (LRU reordering requires write access)
- Server spawns one `std::thread` per client connection — all threads share one KVStore instance safely
- `std::atomic<bool>` flag for clean shutdown signaling across threads
- `std::atomic<uint64_t>` counters for metrics — thread-safe increments with no mutex overhead

---

## Future Improvements

- **Lock striping** — partition KVStore into N shards each with its own mutex, reducing contention under high concurrency
- **Sharded architecture** — multiple KVStore instances across threads for horizontal scalability
- **Async background persistence** — write to disk on a background thread instead of blocking on shutdown
