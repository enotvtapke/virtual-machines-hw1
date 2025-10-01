#include <chrono>
#include <iostream>
#include <random>
#include <thread>
#include "header.h"

constexpr size_t MEMORY_SIZE_BYTES_FOR_STRIDE = 64 * 1024 * 1024;
constexpr int EXPERIMENT_COUNT = 10;


double measure_time_for_stride(const std::vector<uint8_t> &array, const size_t stride = 1) {
    volatile uint8_t dummy = 0;
    const auto start = std::chrono::steady_clock::now();
    for (long long i = 0; i < MEMORY_SIZE_BYTES_FOR_STRIDE; i += stride) {
        dummy = array[i];
    }
    const auto end = std::chrono::steady_clock::now();
    if (dummy == 1) std::cout << "This won't happen" << std::endl;
    const auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    return static_cast<double>(duration.count()) / (array.size() / stride);
}

template<typename T>
void clean(std::vector<T> &initial) {
    const int to_drop = initial.size() / 5;
    std::sort(initial.begin(), initial.end());
    initial.erase(initial.begin(), initial.begin() + to_drop);
    initial.erase(initial.end() - to_drop, initial.end());
}

void cache_line_size_experiment() {
    std::cout << "Cache line size experiment" << std::endl;
    auto memory = std::vector<uint8_t>(MEMORY_SIZE_BYTES_FOR_STRIDE);
    std::iota(memory.begin(), memory.end(), 0);

    for (int stride = 1; stride <= 1 << 9; stride *= 2) {
        const auto measurements = new std::vector<double>(0);
        for (int i = 0; i < EXPERIMENT_COUNT; ++i) {
            measurements->push_back(measure_time_for_stride(memory, stride));
        }
        clean(*measurements);
        std::cout << stride << " Bytes stride  \t| ";
        std::cout << std::accumulate(measurements->begin(), measurements->end(), 0.) / measurements->size() <<
                " ns/access" << std::endl;
        delete measurements;
    }
}
