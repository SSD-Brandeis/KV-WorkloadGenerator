#ifndef WORKLOAD_H
#define WORKLOAD_H

void load_from_file(std::string file_path);

void generate_keys(long insert_count, uint32_t key_size, uint32_t num_preserved_bits);

void generate_non_existing_keys(long max_non_existing_key_count, uint32_t key_size);

int generate(std::unique_ptr<Config>& config);

#endif // WORKLOAD_H