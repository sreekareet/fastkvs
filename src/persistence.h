#ifndef PERSISTENCE_H
#define PERSISTENCE_H

#include<string>
#include "kvstore.h"

class Persistence{
    public:
        Persistence(const std::string &filename);
        void save(const KVStore &store);
        void load(KVStore &store);

    private:
        std::string pfilename;
};

#endif
