#include "kvstore.h"
#include <mutex>

KVStore :: KVStore(size_t capacity):lru(capacity), kv_c(capacity) {}

void KVStore :: put(const std::string &key, const std::string &value){
    std::unique_lock lock(kv_mutex);
    if (kv_map.size() >= kv_c && kv_map.find(key) == kv_map.end()){
        std::string evicted = lru.evict();
        kv_map.erase(evicted);
    }
    kv_map[key] =  value;
    lru.touch(key);
}

bool KVStore :: get(const std::string &key, std::string &value) const{
    std::unique_lock lock(kv_mutex);
    auto it = kv_map.find(key);
    if (it == kv_map.end()){
        return false;
    }
    value = it -> second;
    lru.touch(key);
    return true;
}

void KVStore :: remove(const std::string &key)
{
    std::unique_lock lock(kv_mutex);
    lru.remove(key);
    kv_map.erase(key);

}

std::vector<std::string> KVStore::get_all_keys() const {
    std::shared_lock lock(kv_mutex);
    std::vector<std::string> keys;
    keys.reserve(kv_map.size());
    for (const auto& kv : kv_map) {
        keys.push_back(kv.first);
    }
    return keys;
}

