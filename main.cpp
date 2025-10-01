#include <iostream>
#include <ostream>

#include "header.h"

void setup_affinity(int cpu_id) {
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(cpu_id, &mask);
    if (sched_setaffinity(0, sizeof(cpu_set_t), &mask) != 0) {
        std::cerr << "ERROR: Could not set CPU affinity" << std::endl;
    }
}

int main() {
    setup_affinity(12);
    // cache_size_experiment();
    // std::cout << std::endl;
    cache_line_size_experiment();
    // std::cout << std::endl;
    // cache_assoc_experiment();
}
