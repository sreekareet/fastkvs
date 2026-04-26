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
    assert(store.get("A").has_value());
    assert(store.get("A").value() == "ValueA");

    assert(store.get("B").has_value());
    assert(store.get("B").value() == "ValueB");

    // Add third key -> should evict LRU ("A")
    store.put("C", "ValueC");

    // "A" should be evicted
    assert(!store.get("A").has_value());

    // "B" and "C" should still exist
    assert(store.get("B").has_value() && store.get("B").value() == "ValueB");
    assert(store.get("C").has_value() && store.get("C").value() == "ValueC");

    // Remove "B"
    store.remove("B");
    assert(!store.get("B").has_value());

    std::cout << "KVStore test passed successfully!\n";
    return 0;
}
