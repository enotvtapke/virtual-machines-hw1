#include <iostream>
#include <random>
#include <vector>
#include <chrono>
#include <numeric>
#include <algorithm>
#include <sys/mman.h>
#include <bits/stdc++.h>

constexpr unsigned int REPEATS = 10'000;

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

constexpr double ASSOC_JUMP_THRESHOLD = 0.3;

void cache_assoc_experiment(const int max_memory, const int max_assoc, const int max_stride) {
    printf("%-13s", "stride\\spots");
    for (int i = 1; i <= max_assoc; ++i) {
        printf(",%-3d", i);
    }
    printf("\n");

    int h = 16;
    while (h * max_assoc <= max_memory) {
        int s = 1;
        printf("%-13d", h);
        double prev_time = -1;
        while (s <= max_assoc) {
            const auto current_time = time(h, s);
            if (h == max_stride & prev_time > 0 && current_time - prev_time > current_time * ASSOC_JUMP_THRESHOLD) {
                std::cout << "Entity assoc: " << s - 1 << std::endl;
            }
            printf(",%.0f ", current_time * 10);
            prev_time = current_time;
            ++s;
        }
        printf("\n");
        h *= 2;
        if (h > max_stride) {
            break;
        }
    }
}

std::vector<double> time_for_stride(const int stride, const int spots_num) {
    std::vector<double> times(spots_num);
    for (int spots = 0; spots < spots_num; ++spots) {
        times[spots] = time(stride, spots + 1);
    }
    return times;
}

void print_times(const std::vector<double> &times) {
    for (const double time: times) {
        printf(",%.0f ", time * 10);
    }
    printf("\n");
}

constexpr double CHANGE_THRESHOLD = 0.2;

std::pair<int, int> increases_decreases(const std::vector<double> &times1, const std::vector<double> &times2) {
    assert(times1.size() == times2.size());
    int increases = 0;
    int decreases = 0;
    for (int i = 0; i < times1.size(); ++i) {
        if (times1[i] - times2[i] > times1[i] * CHANGE_THRESHOLD) {
            ++increases;
        } else if (times2[i] - times1[i] > times2[i] * CHANGE_THRESHOLD) {
            ++decreases;
        }
    }
    return std::make_pair(increases, decreases);
}

void cache_line_size_experiment(const int max_memory, const int max_spots, const int max_stride) {
    printf("%-13s,%-5s", "stride\\spots", "inc");
    for (int i = 1; i <= max_spots; ++i) {
        printf(",%-3d", i);
    }
    printf("\n");

    int higher_stride = 16;
    int max_lower_stride = (higher_stride >> 2) * 3;
    int prev_prev_dif = 0;
    int prev_dif = 0;
    while ((higher_stride + max_lower_stride) * max_spots <= max_memory) {

        printf("%-13d", higher_stride);
        const auto times_higher_stride = time_for_stride(higher_stride, max_spots);
        printf(",%-5s", "");
        print_times(times_higher_stride);

        for (int lower_stride = higher_stride >> 2; lower_stride <= max_lower_stride; lower_stride += higher_stride >> 2) {
            printf("%-13d", higher_stride + lower_stride);
            const auto times_higher_lower_stride = time_for_stride(higher_stride + lower_stride, max_spots);
            const auto [increases, decreases] = increases_decreases(times_higher_lower_stride, times_higher_stride);
            const int dif = increases - decreases;
            if (lower_stride == higher_stride >> 1) {
                if (prev_prev_dif > max_spots * 0.1 && dif < max_spots * -0.1) {
                    std::cout << "Entity line size: " << higher_stride / 2 << std::endl;
                }
                prev_prev_dif = prev_dif;
                prev_dif = dif;
            }
            printf(",%-5d", dif);
            print_times(times_higher_lower_stride);
        }

        higher_stride *= 2;
        max_lower_stride = (higher_stride >> 2) * 3;
        if (higher_stride + max_lower_stride > max_stride) {
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
    setup_affinity(12);
    constexpr int max_memory = 512 * 1024 * 1024;
    memory = (char *) mmap(nullptr, max_memory, PROT_READ | PROT_WRITE,MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    const auto original_stdout = stdout;

    auto file = fopen("../cache_line_size_table.csv", "w");
    stdout = file;
    cache_line_size_experiment(max_memory, 2000, 32 * 1024);
    fclose(file);

    file = fopen("../cache_assoc_table.csv", "w");
    stdout = file;
    cache_assoc_experiment(max_memory, 32, 1 * 1024 * 1024);
    fclose(file);

    stdout = original_stdout;
    return 0;
}
