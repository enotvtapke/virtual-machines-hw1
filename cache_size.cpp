#include <chrono>
#include <iostream>
#include <random>
#include <thread>
#include <unistd.h>

int *permutation(const int size) {
    const auto array = static_cast<int *>(aligned_alloc(sysconf(_SC_PAGESIZE), size * sizeof(int)));
    for (int i = 0; i < size; ++i) {
        array[i] = i;
    }
    std::shuffle(&array[0], &array[size - 1],
                 std::default_random_engine(std::chrono::steady_clock::now().time_since_epoch().count()));
    return array;
}

std::pair<double *, size_t> clean(const double *initial, const int size) {
    const int to_drop = size / 5;
    const auto clean_initial = new double[size - to_drop * 2];
    for (int i = to_drop; i < size - to_drop; ++i) {
        clean_initial[i - to_drop] = initial[i];
    }
    return std::pair(clean_initial, size - to_drop * 2);
}

double measure_time_for_size(const int *memory, const int size, const int warmup_count = 100) {
    for (int i = 0; i < warmup_count; ++i) {
        int current_index = 0;
        for (int j = 0; j < size; ++j) {
            current_index = memory[current_index];
        }
    }

    const auto start = std::chrono::steady_clock::now();
    volatile int current_index = 0;
    for (int i = 0; i < size; ++i) {
        current_index = memory[current_index];
    }
    const auto end = std::chrono::steady_clock::now();

    delete[] memory;

    const auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);

    return static_cast<double>(duration.count()) / size;
}

constexpr int EXPERIMENTS_COUNT = 500;
constexpr int MAX_MEMORY_SIZE_IN_BYTES = 256 * 1024;

void cache_size_experiment() {
    std::cout << "Cache size experiment" << std::endl;
    for (int size = 1024 / 4; size < MAX_MEMORY_SIZE_IN_BYTES / 4; size *= 2) {
        double times[EXPERIMENTS_COUNT];
        for (double &time: times) {
            time = measure_time_for_size(permutation(size), size, 100);
        }
        auto [clean_time, clean_time_size] = clean(times, EXPERIMENTS_COUNT);
        std::cout << "Memory size " << size * 4 << " Bytes  \t| " <<
                std::accumulate(&clean_time[0], &clean_time[clean_time_size], 0.) / clean_time_size << " ns/access" << std::endl;
    }
}
