#ifndef KVSTORE_H
#define KVSTORE_H

#include <unordered_map>
#include <string>
#include <string_view>
#include <optional>
#include <vector>
#include <shared_mutex>
#include "lru_cache.h"

/**
 * KVStore
 * Thread-safe key-value store using single mutex locking.
 * All operations are O(1) average.
 * 
 * Thread Safety:
 *   - All public APIs are protected by kv_mutex.
 */
class KVStore{
    public:
        KVStore(size_t capacity);
        void put(const std::string &key, const std::string &value);
        [[nodiscard]] std::optional<std::string> get(std::string_view key) const;
        void remove(const std::string &key);
        std::vector<std::string> get_all_keys() const;

    private:
        mutable LRUCache lru;
        size_t kv_c;
        std::unordered_map<std::string, std::string> kv_map;
        mutable std::shared_mutex kv_mutex; //single coarse-grained lock per KVStore object.
};

#endif
