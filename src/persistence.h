#ifndef PERSISTENCE_H
#define PERSISTENCE_H

#include<string>
#include <filesystem>
#include "kvstore.h"

class Persistence{
    public:
        Persistence(const std::string &filename);
        void save(const KVStore &store);
        void load(KVStore &store);
        // Optional: check file exists before loading
        bool exists() const {
            return std::filesystem::exists(pfilename);
        }

    private:
        std::string pfilename;
};

#endif
