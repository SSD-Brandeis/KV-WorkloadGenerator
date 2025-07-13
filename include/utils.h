#ifndef UTILS_H
#define UTILS_H

#include "config.h"
#include <iomanip>

int parse_args(int argc, char *argv[], std::unique_ptr<Config>& config);

inline void showProgress(uint32_t workload_size, uint32_t counter);

#endif // UTILS_H