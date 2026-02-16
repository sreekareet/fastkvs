
#ifndef LRU_CACHE_H
#define LRU_CACHE_H

#include <string>
#include <unordered_map>
#include <list>

class LRUCache{
    public:
        LRUCache(size_t capacity);
        void touch(const std::string &key);
        std::string evict();
        void remove(const std::string &key);

    private:
        size_t c;
        std::list<std::string> lst; //double linked list
        std::unordered_map<std::string, std::list<std::string>::iterator> cacheMap; //map -> store key and its position
};

#endif
