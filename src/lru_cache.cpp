#include "lru_cache.h"

LRUCache::LRUCache(size_t capacity):c(capacity){}

void LRUCache:: touch(const std::string &key){

    auto it = cacheMap.find(key);

    if (it != cacheMap.end()){  //if key exists
        lst.erase(it -> second);
    }
    lst.push_front(key);
    cacheMap[key] = lst.begin();
}

// When capacity is exceeded, remove the least recently used key.
// The back of the list represents the LRU entry.
std::optional<std::string> LRUCache :: evict(){
        
    if (lst.empty())
        return std::nullopt;

    std::string key = lst.back();
    lst.pop_back();
    cacheMap.erase(key);

    return key;
}

void LRUCache :: remove(const std::string &key){

    auto it = cacheMap.find(key);

    if (it != cacheMap.end()){
        lst.erase(it->second);
        cacheMap.erase(it);
    }
}
