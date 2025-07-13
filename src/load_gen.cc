#include "utils.h"
#include "workload.h"
#include <iostream>

int main(int argc, char *argv[])
{
    std::unique_ptr<Config> config;
    if (parse_args(argc, argv, config))
    {
        exit(1);
    }
    return generate(config);
}