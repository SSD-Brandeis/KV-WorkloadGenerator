#include "key.h"
#include "workload.h"

#include <string>
#include <fstream>
#include <vector>
#include <set>

std::vector<Key> insert_pool;
std::set<Key> global_insert_pool_set;
std::vector<Key> global_insert_pool;
std::set<Key> global_non_existing_key_set;
std::vector<Key> global_non_existing_key_pool;

void load_from_file(std::string file_path) {
    std::ifstream fin(file_path);
    if (fin.good()) {
        
    }
}