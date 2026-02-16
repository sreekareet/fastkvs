#include "persistence.h"
#include <fstream>
#include <iostream>

Persistence::Persistence(const std::string& filename)
    : pfilename(filename) {}

// Save KVStore to file
void Persistence::save(const KVStore& store) {
    std::ofstream out(pfilename, std::ios::trunc);
    if (!out.is_open()) {
        std::cerr << "Error opening file for save.\n";
        return;
    }

    std::string value;
    for (const auto& key : store.get_all_keys()) {
        if (store.get(key, value)) {
            out << key << " " << value << "\n";
        }
    }

    out.close();
}

// Load KVStore from file
void Persistence::load(KVStore& store) {
    std::ifstream in(pfilename);
    if (!in.is_open()) return; // No previous data

    std::string key, value;
    while (in >> key >> value) {
        store.put(key, value);
    }

    in.close();
}
