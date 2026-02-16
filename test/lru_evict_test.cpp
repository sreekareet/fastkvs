#include "../src/kvstore.h"
#include <cassert>
#include <iostream>

int main() {
    std::cout << "Starting KVStore test...\n";

    // Create a store with capacity 2
    KVStore store(2);

    // Add two keys
    store.put("A", "ValueA");
    store.put("B", "ValueB");

    std::string value;

    // Both should exist
    assert(store.get("A", value));
    assert(value == "ValueA");

    assert(store.get("B", value));
    assert(value == "ValueB");

    // Add third key -> should evict LRU ("A")
    store.put("C", "ValueC");

    // "A" should be evicted
    bool existsA = store.get("A", value);
    assert(!existsA);  // A should not exist anymore

    // "B" and "C" should still exist
    assert(store.get("B", value) && value == "ValueB");
    assert(store.get("C", value) && value == "ValueC");

    // Remove "B"
    store.remove("B");
    assert(!store.get("B", value));

    std::cout << "KVStore test passed successfully!\n";
    return 0;
}
