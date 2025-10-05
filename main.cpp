#include <iostream>
#include <random>
#include <vector>
#include <chrono>
#include <numeric>
#include <algorithm>
#include <sys/mman.h>
#include <bits/stdc++.h>

#include "header.h"

constexpr unsigned int REPEATS = 100'000;

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

template <typename INDEX_T, typename DATA_T>
struct Table {
    std::vector<INDEX_T> index_column;
    std::vector<std::vector<DATA_T>> data;

    [[nodiscard]] std::vector<DATA_T> row_by_index(const INDEX_T index) const {
        for (int i = 0; i < index_column.size(); ++i) {
            if (index_column[i] == index) {
                return data[i];
            }
        }
        throw std::runtime_error("Index not found");
    }

    void print(FILE * file = stdout, const bool is_time = true) const {
        for (int i =0; i < index_column.size(); ++i) {
            fprintf(file, "%-13d", index_column[i]);
            for (const double time: data[i]) {
                if (is_time) {
                    fprintf(file, ",%-2.0f ", time * 100);
                } else {
                    fprintf(file, ",%-2.0f ", time);
                }
            }
            fprintf(file, "\n");
        }
    }
};

Table<int, double> cache_assoc_experiment(const int max_memory, const int max_assoc, const int max_stride) {
    std::vector<std::vector<double>> times{};
    std::vector<int> strides{};
    int stride = 16;
    while (stride * max_assoc <= max_memory) {
        int spots_num = 1;
        std::vector<double> times_for_stride{};
        while (spots_num <= max_assoc) {
            const auto current_time = time(stride, spots_num);
            times_for_stride.push_back(current_time);
            ++spots_num;
        }
        times.push_back(times_for_stride);
        strides.push_back(stride);
        stride *= 2;
        if (stride > max_stride) {
            break;
        }
    }
    return {strides, times};
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

constexpr double LOWER_STRIDE_TIME_CHANGE_THRESHOLD = 0.2;

std::pair<int, int> increases_decreases(const std::vector<double> &times1, const std::vector<double> &times2) {
    assert(times1.size() == times2.size());
    int increases = 0;
    int decreases = 0;
    for (int i = 0; i < times1.size(); ++i) {
        if (times1[i] - times2[i] > times1[i] * LOWER_STRIDE_TIME_CHANGE_THRESHOLD) {
            ++increases;
        } else if (times2[i] - times1[i] > times2[i] * LOWER_STRIDE_TIME_CHANGE_THRESHOLD) {
            ++decreases;
        }
    }
    return std::make_pair(increases, decreases);
}

Table<int, double> cache_line_size_experiment(const int max_memory, const int max_spots, const int max_stride) {
    Table<int, double> table{{}, {}};

    int higher_stride = 16;
    int lower_stride = higher_stride / 2;
    while ((higher_stride + lower_stride) * max_spots <= max_memory) {
        const auto times_higher_stride = time_for_stride(higher_stride, max_spots);
        const auto times_higher_lower_stride = time_for_stride(higher_stride + lower_stride, max_spots);

        table.index_column.push_back(higher_stride);
        table.data.push_back(times_higher_stride);
        table.index_column.push_back(higher_stride + lower_stride);
        table.data.push_back(times_higher_lower_stride);

        higher_stride *= 2;
        lower_stride = higher_stride / 2;
        if (higher_stride + lower_stride > max_stride) {
            break;
        }
    }
    return table;
}

constexpr double CACHE_SIZE_JUMP_THRESHOLD = 0.4;

void setup_affinity(const int cpu_id) {
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(cpu_id, &mask);
    if (sched_setaffinity(0, sizeof(cpu_set_t), &mask) != 0) {
        std::cerr << "ERROR: Could not set CPU affinity" << std::endl;
    }
}

void print_vector(const std::vector<size_t>& data) {
    for (const auto d: data) {
        printf("%lu ", d);
    }
    printf("\n");
}

// for assoc in assocs:
//     i: int = 0
//     for stride, jumps in reversed(list(zip(strides, jumps_per_stride))):
//         if all(jump != assoc for jump in jumps) and any((assoc * 1.8 < jump < assoc * 2.2) for jump in jumps):
//             print(f'Entity with assoc {assoc} has entity stride {stride * 2} Bytes and size {stride * 2 * assoc} Bytes.')
//             break

void analyze_jumps_for_assoc(const Table<int, size_t>& data) {
    const std::vector<size_t> assocs = data.data.back();
    for (const auto assoc: assocs) {
        for (int i = data.index_column.size() - 1; i >= 0; --i) {
            if (std::ranges::all_of(data.data[i], [&](const size_t jump) { return jump != assoc; }) &&
                std::ranges::any_of(data.data[i], [&](const size_t jump) { return similar(jump, assoc * 2); })) {
                printf("Entity with assoc %lu has entity stride %d Bytes and size %lu Bytes\n",
                       assoc, data.index_column[i] * 2, assoc * 2 * data.index_column[i]);
                break;
            }
        }
    }
}

void analyze_jumps_for_line_size(const Table<int, size_t>& data) {
    for (int i = 0; i < data.index_column.size() - 3; i += 2) {
        if (!data.data[i].empty() && !data.data[i + 1].empty() && similar(data.data[i][0], data.data[i + 1][0], 0.4) &&
            !data.data[i + 2].empty() && !data.data[i + 3].empty() && data.data[i + 3][0] / data.data[i + 2][0] > 1.7) {
            printf("Entity has line size %d\n", data.index_column[i]);
        }
    }
}

void print_header(int size, FILE *file = stdout) {
    fprintf(file, "%-13s", "stride\\spots");
    for (int i = 1; i <= size; ++i) {
        fprintf(file, ",%-3d", i);
    }
    fprintf(file, "\n");
}

int main() {
    setup_affinity(0);
    constexpr int max_memory = 512 * 1024 * 1024;
    memory = (char *) mmap(nullptr, max_memory, PROT_READ | PROT_WRITE,MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    {
        constexpr int max_spots = 2000;

        const auto times = cache_line_size_experiment(max_memory, max_spots, 32 * 1024);
        auto file = fopen("./cache_line_size_table.csv", "w");
        print_header(max_spots, file);
        times.print(file);
        fclose(file);

        std::vector<std::vector<size_t>> jumps;
        for (int i = 0; i < times.index_column.size(); ++i) {
            jumps.push_back(jumpIndices(times.data[i], 4, 1.3, 0.1, 4));
        }
        const Table jump_table(times.index_column, jumps);
        file = fopen("./cache_line_size_jump_table.csv", "w");
        jump_table.print(file, false);
        fclose(file);
        analyze_jumps_for_line_size(jump_table);
    }
    {
        const auto times = cache_assoc_experiment(max_memory, 100, 1 * 1024 * 1024);
        auto file = fopen("./cache_assoc_table.csv", "w");
        print_header(100, file);
        times.print(file);
        fclose(file);

        std::vector<std::vector<size_t>> jumps;
        for (int i = 0; i < times.index_column.size(); ++i) {
            jumps.push_back(jumpIndices(times.data[i], 4, 1.3, 0.1, 4));
        }
        const Table jump_table(times.index_column, jumps);
        file = fopen("./cache_assoc_jump_table.csv", "w");
        jump_table.print(file, false);
        fclose(file);
        analyze_jumps_for_assoc(jump_table);
    }
    return 0;
}