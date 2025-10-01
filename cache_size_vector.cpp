#include <chrono>
#include <iostream>
#include <random>
#include <thread>

template<typename T>
void clean(std::vector<T> &initial) {
    const int to_drop = initial.size() / 5;
    std::sort(initial.begin(), initial.end());
    initial.erase(initial.begin(), initial.begin() + to_drop);
    initial.erase(initial.end() - to_drop, initial.end());
}

double measure_time_for_size(const std::vector<int> &array, const int warmup_count = 100) {
    for (int i = 0; i < warmup_count; ++i) {
        int current_index = 0;
        for (size_t j = 0; j < array.size(); ++j) {
            current_index = array[current_index];
        }
    }
    volatile int current_index = 0;
    const auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < array.size(); ++i) {
        current_index = array[current_index]; // Chase the pointer
    }
    const auto end = std::chrono::steady_clock::now();
    const auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    return static_cast<double>(duration.count()) / array.size();
}

constexpr int EXPERIMENTS_COUNT = 100;

void cache_size_experiment() {
    const auto aligned_mem = static_cast<int*>(aligned_alloc(sysconf(_SC_PAGESIZE), 2 * 1024 * 1024));
    for (int size = 1024; size <= 2 * 1024 * 1024; size *= 2) {
        auto memory = std::vector(aligned_mem, aligned_mem + size);
        std::iota(memory.begin(), memory.end(), 0);
        std::shuffle(
            memory.begin(),
            memory.end(),
            std::default_random_engine(std::chrono::steady_clock::now().time_since_epoch().count())
        );

        const auto measurements = new std::vector<double>(0);
        for (int i = 0; i < EXPERIMENTS_COUNT; ++i) {
            measurements->push_back(measure_time_for_size(memory, 100));
        }
        clean(*measurements);
        std::cout << size * 4 << " Bytes memory size  \t| ";
        std::cout << std::accumulate(measurements->begin(), measurements->end(), 0.) / measurements->size() <<
                " ns/access" << std::endl;
        delete measurements;
    }
    delete[] aligned_mem;
}

int main() {
    cache_size_experiment();

    return 0;
}
