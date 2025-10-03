#include <iostream>
#include <random>
#include <vector>
#include <chrono>
#include <numeric>
#include <algorithm>
#include <sys/mman.h>

constexpr unsigned int REPEATS = 10'000'000;

inline std::mt19937& get_random_engine() {
    static std::mt19937 engine(std::random_device{}());
    return engine;
}

char* memory;

double time(const int stride, const int spots_num) {
    std::vector<int> indices(spots_num);
    std::iota(indices.begin(), indices.end(), 0);
    std::ranges::shuffle(indices, get_random_engine());

    for (int i = 0; i < spots_num - 1; ++i) {
        * (void **) (memory + indices[i] * stride) = memory + indices[i + 1] * stride;
    }
    * (void **) (memory + indices[spots_num - 1] * stride) = memory + indices[0] * stride;

    // Warmup
    auto current = (void *) memory;
    for (unsigned int i = 0; i < REPEATS; ++i) {
        current = * (void **) current;
    }

    current = (void *) memory;
    const auto start_time = std::chrono::high_resolution_clock::now();
    for (unsigned int i = 0; i < REPEATS; ++i) {
        current = * (void **) current;
    }
    const auto end_time = std::chrono::high_resolution_clock::now();

    if (current == (void *) 256) {
        std::cout << "Impossible!" << std::endl;
    }
    return static_cast<double>((end_time - start_time).count()) / REPEATS;
}

void cache_assoc_experiment_new(const int max_memory, const int max_assoc, const int max_stride) {
    printf("%-13s", "stride\\spots");
    for (int i = 1; i <= max_assoc; ++i) {
        printf("%-3d", i);
    }
    printf("\n");

    int h = 16;
    while (h * max_assoc <= max_memory) {
        int s = 1;
        printf("%-13d", h);
        while (s <= max_assoc) {
            const auto current_time = time(h, s);
            printf("%.0f ", current_time * 10);
            ++s;
        }
        printf("\n");
        h *= 2;
        if (h > max_stride) {
            break;
        }
    }
}

void setup_affinity(int cpu_id) {
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(cpu_id, &mask);
    if (sched_setaffinity(0, sizeof(cpu_set_t), &mask) != 0) {
        std::cerr << "ERROR: Could not set CPU affinity" << std::endl;
    }
}

int main() {
    setup_affinity(1);
    constexpr int max_memory = 512 * 1024 * 1024;
    memory = (char *) mmap(nullptr, max_memory, PROT_READ | PROT_WRITE,MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    cache_assoc_experiment_new(max_memory, 30, 4 * 1024 * 1024);
    return 0;
}
