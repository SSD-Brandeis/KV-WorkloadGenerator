/* *  Created on: September 05, 2019
 *  Author: Subhadeep
 */

// #include <cstdio>
#include <iostream>
#include <sstream>
#include <set>
#include <vector>
#include <string>
#include <random>
#include <chrono>
#include <cmath>
#include <algorithm>
#include <fstream>
#include <cmath>
#include <iomanip>
#include "args.hxx"
#include "generator.h"
#include "key.h"
#include "buffer_output.h"

const char value_alphanum[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"; // "0123456789";
std::vector<Key> insert_pool;
std::set<Key> global_insert_pool_set;
std::vector<Key> global_insert_pool;
std::set<Key> global_non_existing_key_set;
std::vector<Key> global_non_existing_key_pool;
int num_insert_key_prefix = 62 * 62;
Generator *insertIndexGenerator = nullptr;
bool sorted = false;
Generator *updateIndexGenerator = nullptr;
Generator *nonExistingPointLookupIndexGenerator = nullptr;
Generator *existingPointLookupIndexGenerator = nullptr;
int get_choice(long, long, long, long, long, long, long, long, long, long, long, long, long);
void generate_workload();
void print_workload_parameters(int _insert_count, int _update_count, int _point_delete_count, int _range_delete_count, int _effective_ingestion_count);
std::string get_value(int _value_size);
inline float get_range_query_selectivity() {
    if (!enable_ycsb_capped_range_queries) {
        return range_query_selectivity;
    }
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(0.0f, range_query_selectivity);
    return dis(gen);
}
std::string get_value(int _value_size)
{
    char *s = new char[(int)_value_size + 1];
    for (int i = 0; i < _value_size; ++i)
    {
        s[i] = value_alphanum[rand() % (sizeof(value_alphanum) - 1)];
    }
    s[_value_size] = '\0';
    std::string value(s);  // (shubham): fixes dangling pointer for s
    delete[] s;
    return value;
}
std::vector<std::string> StringSplit(const std::string &arg, char delim)
{
    std::vector<std::string> splits;
    std::stringstream ss(arg);
    std::string item;
    while (std::getline(ss, item, delim))
    {
        splits.push_back(item);
    }
    return splits;
}
void generate_inserts(size_t insert_count, size_t key_size, uint32_t num_preserved_bits) {
    global_insert_pool.reserve(insert_count);
    std::unordered_set<Key> unique_keys;
    unique_keys.reserve(insert_count);
    size_t batch_size = std::min<size_t>(100000, insert_count);
    size_t generated = 0;
    while (generated < insert_count) {
        std::vector<Key> batch;
        batch.reserve(batch_size);
        for (size_t i = 0; i < batch_size && generated < insert_count; ++i) {
            uint32_t index = insertIndexGenerator->getNext();
            Key key;
            if (STRING_KEY_ENABLED) {
                char prefix[3];
                prefix[0] = Key::key_alphanum[(index / 62) % 62];
                prefix[1] = Key::key_alphanum[index % 62];
                prefix[2] = '\0';
                Key key_suffix = Key::get_key(key_size - 2, STRING_KEY_ENABLED);
                key = Key(prefix) + key_suffix;
            } else {
                Key key_suffix = Key::get_key(32 - num_preserved_bits, STRING_KEY_ENABLED);
                index <<= (32 - num_preserved_bits);
                key = Key(key_suffix.key_int32_ | index);
            }
            batch.push_back(key);
        }
        std::unordered_set<Key> batch_set(batch.begin(), batch.end());
        for (const auto &key : batch_set) {
            if (unique_keys.insert(key).second) {
                global_insert_pool.push_back(key);
                ++generated;
                if (generated >= insert_count) {
                    break;
                }
            }
        }
    }
}
void generate_non_existing_keys(size_t max_non_existing_count, size_t key_size) {
    global_non_existing_key_pool.reserve(max_non_existing_count);
    std::unordered_set<Key> unique_keys(global_insert_pool.begin(), global_insert_pool.end());
    std::unordered_set<Key> non_existing_keys;
    while (non_existing_keys.size() < max_non_existing_count) {
        Key key = Key::get_key(key_size, STRING_KEY_ENABLED);
        if (unique_keys.find(key) == unique_keys.end() && non_existing_keys.insert(key).second) {
            global_non_existing_key_pool.push_back(key);
        }
    }
    std::sort(global_non_existing_key_pool.begin(), global_non_existing_key_pool.end());
}
void generate_workload()
{
    long total_operation_count = insert_count + update_count + point_delete_count + range_delete_count + point_query_count + range_query_count;
    std::cout << "Total operation count = " << total_operation_count << std::endl
              << std::flush;
    std::set<Key> tmp_insert_pool_set;
    std::vector<Key> tmp_insert_pool_vec;
    if (load_from_existing_workload)
    {
        std::ifstream fin(file_path + FILENAME);
        if (fin.good())
        {
            std::vector<std::string> splits;
            std::string line;
            while (getline(fin, line))
            {
                splits = StringSplit(line, ' ');
                Key key = splits[1];
                if (!STRING_KEY_ENABLED)
                {
                    key = atoi(splits[1].c_str());
                }
                if (tmp_insert_pool_set.find(key) == tmp_insert_pool_set.end())
                {
                    tmp_insert_pool_set.insert(key);
                    global_insert_pool.push_back(key);
                    insert_pool.push_back(key);
                }
            }
            sorted = false;
        }
        fin.close();
    }
    if (insert_count + insert_pool.size() < point_delete_count + range_delete_count * range_delete_selectivity * insert_count)
    {
        std::cout << "\033[1;31m ERROR:\033[0m insert_count < point_delete_count + range_delete_count * range_delete_selectivity * insert_count" << std::endl;
        exit(0);
    }
    std::string output_filename = "";
    if (out_filename.compare("") == 0)
    {
        output_filename = file_path + FILENAME;
    }
    else
    {
        output_filename = out_filename;
    }
    BufferOutput fp(output_filename);
    long _insert_count = 0;
    long _update_count = 0;
    long _point_delete_count = 0;
    long _range_delete_count = 0;
    long _point_query_count = 0;
    long _non_existing_point_query_count = 0;
    long _existing_point_query_count = 0;
    long _range_query_count = 0;
    bool _positive_direction = false;
    long _total_operation_count = 0;
    long _effective_ingestion_count = 0; // insert = +1 ; update = 0 ; point_delete = -1 ; range_delete = -x
    int flag = 0;
    std::tuple<long, long> _last_range_query = std::make_tuple(0, 0);
    uint32_t num_char = (std::string(Key::key_alphanum)).size();
    uint32_t num_preserved_bits = 10;
    if (STRING_KEY_ENABLED)
    {
        insertIndexGenerator = new Generator(insert_dist, 0, num_char * num_char - 1, insert_norm_mean_percentile * num_char * num_char, insert_norm_stddev * num_char, insert_beta_alpha, insert_beta_beta, insert_zipf_alpha, num_char * num_char);
    }
    else
    {
        uint32_t int32_preserved_insert_domain_size = pow(2, num_preserved_bits);
        insertIndexGenerator = new Generator(insert_dist, 0, int32_preserved_insert_domain_size - 1, insert_norm_mean_percentile * int32_preserved_insert_domain_size, insert_norm_stddev * int32_preserved_insert_domain_size, insert_beta_alpha, insert_beta_beta, insert_zipf_alpha, int32_preserved_insert_domain_size);
    }
    generate_inserts(insert_count, key_size, num_preserved_bits);
    generate_non_existing_keys(maximum_unique_non_existing_point_query_count, key_size);
    _insert_count = 0;

    double scaling_ratio = 1.0;
    if (STRING_KEY_ENABLED)
        scaling_ratio = num_char;
    nonExistingPointLookupIndexGenerator = new Generator(non_existing_point_lookup_dist, 0, global_non_existing_key_pool.size() - 1, non_existing_point_lookup_norm_mean_percentile * global_non_existing_key_pool.size(), non_existing_point_lookup_norm_stddev * global_non_existing_key_pool.size() / scaling_ratio, non_existing_point_lookup_beta_alpha, non_existing_point_lookup_beta_beta, non_existing_point_lookup_zipf_alpha, global_non_existing_key_pool.size());
    std::vector<int> update_global_index_mapping;
    if (update_count > 0)
    {
        updateIndexGenerator = new Generator(update_dist, 0, global_insert_pool.size() - 1, update_norm_mean_percentile * global_insert_pool.size(), update_norm_stddev * global_insert_pool.size() / scaling_ratio, update_beta_alpha, update_beta_beta, update_zipf_alpha, global_insert_pool.size(), update_global_index_mapping);
    }
    while (_total_operation_count < total_operation_count)
    {
        int choice = get_choice(insert_pool.size(), insert_count, update_count, point_delete_count, range_delete_count, point_query_count, range_query_count, _insert_count, _update_count, _point_delete_count, _range_delete_count, _point_query_count, _range_query_count);
        if (choice == 0)
            continue;
        else if (choice == 1)
        { // INSERT
            long global_insert_pool_size = global_insert_pool.size();
            long index = (long)(rand() % (global_insert_pool_size - _insert_count));
            Key key = global_insert_pool[index + _insert_count];
            // swap the key with the first element after inserted ones
            global_insert_pool[index + _insert_count] = global_insert_pool[_insert_count];
            global_insert_pool[_insert_count] = key;
            Key value = get_value(entry_size - key_size);
            if (sorted)
            {
                std::vector<Key>::iterator it = std::upper_bound(insert_pool.begin(), insert_pool.end(), key);
                insert_pool.insert(it, key);
            }
            else
            {
                insert_pool.push_back(key);
            }
            global_insert_pool_set.insert(key);
            fp << "I " << key << " " << value << std::endl;
            _insert_count++;
            _effective_ingestion_count++;
            _total_operation_count++;
        }
        else if (choice == 2)
        { // UPDATE
            std::vector<int> index_mapping;
            if (!sorted)
            {
                if (existing_point_lookup_dist == 1)
                {
                    std::cout << "sort here" << std::endl;
                    sort(insert_pool.begin(), insert_pool.end());
                    double scaling_ratio = 1.0;
                    sorted = true;
                    if (STRING_KEY_ENABLED)
                        scaling_ratio = num_char;
                    if (updateIndexGenerator != nullptr)
                    {
                        std::cout << "renew update generator" << std::endl;
                        index_mapping = updateIndexGenerator->index_mapping;
                        delete updateIndexGenerator;
                        updateIndexGenerator = new Generator(update_dist, 0, global_insert_pool.size() - 1, update_norm_mean_percentile * global_insert_pool.size(), update_norm_stddev * global_insert_pool.size() / scaling_ratio, update_beta_alpha, update_beta_beta, update_zipf_alpha, global_insert_pool.size(), update_global_index_mapping);
                    }
                }
            }
            long index = updateIndexGenerator->getNext();
            if (index >= (int)insert_pool.size())
            { // Generate an insert instead here
                Key key = global_insert_pool[index];
                global_insert_pool[index] = global_insert_pool[_insert_count];
                global_insert_pool[_insert_count] = key;
                Key value = get_value(entry_size - key_size);
                if (sorted)
                {
                    std::vector<Key>::iterator it = std::upper_bound(insert_pool.begin(), insert_pool.end(), key);
                    insert_pool.insert(it, key);
                }
                else
                {
                    insert_pool.push_back(key);
                }
                global_insert_pool_set.insert(key);
                fp << "I " << key << " " << value << std::endl;
                _insert_count++;
                _effective_ingestion_count++;
            }
            else
            {
                Key key = insert_pool[index];
                Key value = get_value(entry_size - key_size);
                fp << "U " << key << " " << value << std::endl;
                _update_count++;
            }
            _total_operation_count++;
        }
        else if (choice == 3)
        { // POINT DELETE
            // the following if block ensures that all updates are completed before a database is emptied ( in cases where insert_count == point_delete_count)
            if (insert_count == point_delete_count && _insert_count == insert_count && _point_delete_count == point_delete_count - 1 && _update_count < update_count)
            {
                std::cout << "pausing delete to facilitate all remaining updates ... " << std::endl;
            }
            else
            {
                long insert_pool_size = insert_pool.size();
                long index = (long)(rand() % insert_pool_size);
                Key key = insert_pool[index];
                global_insert_pool_set.erase(key);
                insert_pool.erase(insert_pool.begin() + index);
                std::vector<int> index_mapping;
                if (updateIndexGenerator != nullptr && update_count > 0)
                {
                    index_mapping = updateIndexGenerator->index_mapping;
                    delete updateIndexGenerator;
                    updateIndexGenerator = new Generator(update_dist, 0, global_insert_pool.size() - 1, update_norm_mean_percentile * global_insert_pool.size(), update_norm_stddev * global_insert_pool.size() / scaling_ratio, update_beta_alpha, update_beta_beta, update_zipf_alpha, global_insert_pool.size(), update_global_index_mapping);
                }
                index_mapping.clear();
                if (existingPointLookupIndexGenerator != nullptr && point_query_count > 0)
                {
                    index_mapping = existingPointLookupIndexGenerator->index_mapping;

                    delete existingPointLookupIndexGenerator;
                    existingPointLookupIndexGenerator = new Generator(existing_point_lookup_dist, 0, insert_pool.size() - 1, existing_point_lookup_norm_mean_percentile * insert_pool.size(), existing_point_lookup_norm_stddev * insert_pool.size() / scaling_ratio, existing_point_lookup_beta_alpha, existing_point_lookup_beta_beta, existing_point_lookup_zipf_alpha, insert_pool.size(), index_mapping);
                }
                fp << "D " << key << " " << std::endl;
                _point_delete_count++;
                _effective_ingestion_count--;
                _total_operation_count++;
            }
        }
        else if (choice == 4)
        { // RANGE DELETE
            // selectivity is computed on the current size of the insert pool (insert_pool.size()) and NOT the total inserts to be made (insert_count)
            // the following code-block generates range selectivity as a random number
            // for now we use the hardcoded range selectivity
            long insert_pool_size = insert_pool.size();
            long entries_in_range_delete = -1;
            if ((float)range_delete_selectivity * insert_pool_size > 0 && (float)range_delete_selectivity * insert_pool_size < 1) // computed on the current size of insert pool
                entries_in_range_delete = 1;
            else
                entries_in_range_delete = floor((float)range_delete_selectivity * insert_pool_size); // computed on the current size of insert pool
            long start_index = (long)(rand() % insert_pool_size);
            long end_index = -1;
            if (start_index + entries_in_range_delete > insert_pool_size)
            {
                start_index -= (start_index + entries_in_range_delete - insert_pool_size);
            }
            end_index = start_index + entries_in_range_delete - 1;
            if (start_index < 0 || entries_in_range_delete == 0)
            {
                std::cout << "not enough entries in tree for range delete -- skipping ... ; insert_pool_size = " << insert_pool_size << std::endl;
                std::cout << "start_index = " << start_index << " ; entries_in_range_delete = " << entries_in_range_delete << std::endl;
                flag++;
                if (flag > 20)
                    exit(-1);
            }
            else
            {
                sort(insert_pool.begin(), insert_pool.end());
                Key start_key = insert_pool[start_index];
                Key end_key = insert_pool[end_index];
                for (int i = start_index; i < end_index + 1; i++)
                {
                    global_insert_pool_set.erase(insert_pool[start_index]);
                }
                insert_pool.erase(insert_pool.begin() + start_index, insert_pool.begin() + end_index + 1);

                fp << "R " << start_key << " " << end_key << std::endl;
                _range_delete_count++;
                _effective_ingestion_count -= entries_in_range_delete;
                _total_operation_count++;
            }
        }
        else if (choice == 5)
        { // POINT QUERY
            // the following if block ensures that all point queries are completed before a database is emptied ( in cases where insert_count == point_delete_count)
            if (insert_count == point_delete_count && _insert_count == insert_count && _point_delete_count == point_delete_count - 1 && _point_query_count < point_query_count)
            {
                std::cout << "pausing delete to facilitate all remaining point queries ... " << std::endl;
            }
            else
            {
                float query_type = (float)rand() / RAND_MAX;
                if (query_type <= zero_result_point_lookup_proportion && _non_existing_point_query_count < non_existing_point_query_count)
                {
                    Key key = global_non_existing_key_pool[nonExistingPointLookupIndexGenerator->getNext()];
                    fp << "Q " << key << std::endl;
                    _point_query_count++;
                    _non_existing_point_query_count++;
                    _total_operation_count++;
                }
                else if (_existing_point_query_count < existing_point_query_count)
                {
                    std::vector<int> index_mapping;
                    if (!sorted)
                    {
                        if (existing_point_lookup_dist == 1)
                        {
                            sort(insert_pool.begin(), insert_pool.end());
                            if (STRING_KEY_ENABLED)
                                scaling_ratio = num_char;
                        }
                        if (existing_point_lookup_dist != 0 && existingPointLookupIndexGenerator != nullptr)
                        {
                            index_mapping = existingPointLookupIndexGenerator->index_mapping;
                            delete existingPointLookupIndexGenerator;
                            existingPointLookupIndexGenerator = nullptr;
                            sorted = true;
                        }
                    }
                    if (existingPointLookupIndexGenerator == nullptr)
                    {
                        existingPointLookupIndexGenerator = new Generator(existing_point_lookup_dist, 0, insert_pool.size() - 1, existing_point_lookup_norm_mean_percentile * insert_pool.size(), existing_point_lookup_norm_stddev * insert_pool.size() / scaling_ratio, existing_point_lookup_beta_alpha, existing_point_lookup_beta_beta, existing_point_lookup_zipf_alpha, insert_pool.size(), index_mapping);
                    }
                    long index = 0;
                    if (existing_point_lookup_dist == 0)
                    {
                        index = rand() % insert_pool.size();
                    }
                    else
                    {
                        index = (long)(existingPointLookupIndexGenerator->getNext());
                    }
                    Key key = insert_pool[index];
                    fp << "Q " << key << std::endl;
                    _point_query_count++;
                    _existing_point_query_count++;
                    _total_operation_count++;
                }
            }
        }
        else if (choice == 6)
        { // RANGE QUERY
            // selectivity is computed on the current size of the insert pool (insert_pool.size()) and NOT the total inserts to be made (insert_count)
            // the following code-block generates range selectivity as a random number
            // for now we use the hardcoded range selectivity
            long insert_pool_size = insert_pool.size();
            auto rq_selectivity_ = get_range_query_selectivity();
            long entries_in_range_query = floor(rq_selectivity_ * insert_pool_size); // computed on the current size of insert pool
            long start_index = (long)(rand() % (insert_pool_size - entries_in_range_query));
            long end_index = -1;
            end_index = start_index + entries_in_range_query - 1;
            if (range_query_overlapping_count > 0) {
                if (std::get<0>(_last_range_query) != 0 && _range_query_count % range_query_overlapping_count != 0){
                    start_index = std::get<0>(_last_range_query);
                    end_index = std::get<1>(_last_range_query);
                    if (range_query_overlapping_percent != 1) {
                        long _num_keys_in_range = end_index - start_index;
                        long _shift_index_by = (long)(_num_keys_in_range * (1 - range_query_overlapping_percent));
                        if ((start_index - _shift_index_by) < 0 && !_positive_direction) {
                            _positive_direction = true;
                        } else if ((start_index + _shift_index_by + _num_keys_in_range) > insert_pool_size && _positive_direction) {
                            _positive_direction = false;
                        }
                        if (_positive_direction) {
                            start_index += _shift_index_by;
                        } else {
                            start_index -= _shift_index_by;
                        }
                        end_index = start_index + _num_keys_in_range;
                        _last_range_query = std::make_tuple(start_index, end_index);
                    }
                } else {
                    _last_range_query = std::make_tuple(start_index, end_index);
                }
            }
            if (start_index < 0 || entries_in_range_query == 0)
            {
                std::cout << "not enough entries in tree for range query -- skipping ... ; insert_pool_size = " << insert_pool_size << std::endl;
                std::cout << "start_index = " << start_index << " ; entries_in_range_query = " << entries_in_range_query << std::endl;
                flag++;
                if (flag > 20)
                    {} // exit(-1);
            }
            else
            {
                if (!sorted)
                {
                    sort(insert_pool.begin(), insert_pool.end());
                    sorted = true; //if the number of range queries increases by a lot, this might be a problem in terms of execution speed!!!
                }
                Key start_key = insert_pool[start_index];
                Key end_key = insert_pool[end_index];
                fp << "S " << start_key << " " << end_key << std::endl;
                _range_query_count++;
                _total_operation_count++;
            }
        }
    }
    // print_workload_parameters(_insert_count, _update_count, _point_delete_count, _range_delete_count, _effective_ingestion_count);
}

int get_choice(long insert_pool_size, long insert_count, long update_count, long point_delete_count, long range_delete_count, long point_query_count, long range_query_count, long _insert_count, long _update_count, long _point_delete_count, long _range_delete_count, long _point_query_count, long _range_query_count)
{
    long total_operation_count = (insert_count - _insert_count) + (update_count - _update_count) + (point_delete_count - _point_delete_count) + (range_delete_count - _range_delete_count) + (point_query_count - _point_query_count) + (range_query_count - _range_query_count);
    if (total_operation_count == 0)
        return 0;
    float insert_fraction = (float)(insert_count - _insert_count) / total_operation_count;
    float update_fraction = (float)(update_count - _update_count) / total_operation_count;
    float point_delete_fraction = (float)(point_delete_count - _point_delete_count) / total_operation_count;
    float range_delete_fraction = (float)(range_delete_count - _range_delete_count) / total_operation_count;
    float point_query_fraction = (float)(point_query_count - _point_query_count) / total_operation_count;
    float range_query_fraction = (float)(range_query_count - _range_query_count) / total_operation_count;
    // float cumulative_fraction = insert_fraction + update_fraction + point_delete_fraction + range_delete_fraction + point_query_fraction + range_query_fraction;
    // int choice_domain = 6;
    int choice = 0;

    float rand_float = static_cast<float>(rand()) / (static_cast<float>(RAND_MAX));
    // std::cout << cumulative_fraction << " " << rand_float << std::endl;

    if (rand_float < insert_fraction)
        choice = 1;
    else if (rand_float < insert_fraction + update_fraction)
        choice = 2;
    else if (rand_float < insert_fraction + update_fraction + point_delete_fraction)
        choice = 3;
    else if (rand_float < insert_fraction + update_fraction + point_delete_fraction + range_delete_fraction)
        choice = 4;
    else if (rand_float < insert_fraction + update_fraction + point_delete_fraction + range_delete_fraction + point_query_fraction)
        choice = 5;
    else if (rand_float <= insert_fraction + update_fraction + point_delete_fraction + range_delete_fraction + point_query_fraction + range_query_fraction)
        choice = 6;

    // std::cout << "choice = " << choice << std::endl;
    switch (choice)
    {
    case 6:
        if (_range_query_count < range_query_count && insert_pool_size > 0 && _insert_count >= RQ_THRESHOLD * insert_count)
            break;
        choice--;
        [[fallthrough]];
    case 5:
        if (_point_query_count < point_query_count && insert_pool_size > 0 && _insert_count >= PQ_THRESHOLD * insert_count)
            break;
        // choice = (choice + 1)%choice_domain;
        choice--;
        [[fallthrough]];
    case 4:
        if (_range_delete_count < range_delete_count && insert_pool_size > 0 && _insert_count >= RD_THRESHOLD * insert_count)
            break;
        choice--;
        [[fallthrough]];
    case 3:
        if (_point_delete_count < point_delete_count && insert_pool_size > 0 && _insert_count >= PD_THRESHOLD * insert_count)
            break;
        // choice = (choice + 1)%choice_domain;
        choice--;
        [[fallthrough]];
    case 2:
        if (_update_count < update_count && insert_pool_size > 0 && _insert_count >= U_THRESHOLD * insert_count)
            break;
        // choice = (choice + 1)%choice_domain;
        choice--;
        [[fallthrough]];
    case 1: // for inserts
        if (_insert_count < insert_count)
            break;
        // choice = (choice + 1)%choice_domain;
        choice = 0;
    }

    return choice;
}
