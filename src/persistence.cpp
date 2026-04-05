#include "persistence.h"
#include <fstream>
#include <iostream>

Persistence::Persistence(const std::string& filename)
    : pfilename(filename) {}

// Snapshot-based persistence.
// Rewrites full store to disk.
// Not crash-consistent but simple and predictable.

// Save KVStore to file
void Persistence::save(const KVStore& store) {
    std::ofstream out(pfilename, std::ios::trunc);
    if (!out.is_open()) {
        std::cerr << "Error opening file for save.\n";
        return;
    }

    std::string value;
    for (const auto& key : store.get_all_keys()) {
        if (auto value = store.get(key)) {
            out << key << " " << *value << "\n";
        }
    }
}

// Load KVStore from file
void Persistence::load(KVStore& store) {
    if (!std::filesystem::exists(pfilename)) return;
    std::ifstream in(pfilename);
    std::string key, value;
    while (in >> key >> value) {
        store.put(key, value);
    }
}
