#ifndef KVSTORE_H
#define KVSTORE_H

#include<unordered_map>
#include<string>
#include<vector>
#include<shared_mutex>
#include "lru_cache.h"

class KVStore{
    public:
        KVStore(size_t capacity);
        void put(const std::string &key, const std::string &value);
        bool get(const std::string &key,  std::string &value) const;
        void remove(const std::string&key);
        std::vector<std::string> get_all_keys() const;

    private:
        mutable LRUCache lru;
        size_t kv_c;
        std::unordered_map<std::string, std::string> kv_map;
        mutable std::shared_mutex kv_mutex;
};

#endif
