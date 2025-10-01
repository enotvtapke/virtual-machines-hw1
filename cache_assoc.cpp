#include <iostream>
#include <vector>
#include <chrono>
#include <numeric>
#include <algorithm>
#include <iomanip>

#include "header.h"

constexpr int MAX_ASSOCIATIVITY_TO_TEST = 16;
constexpr size_t PROBE_STRIDE = 64 * 1024;
constexpr long long TOTAL_ACCESSES = 100 * 1000 * 1000;

struct Node {
    Node* next;
};

void cache_assoc_experiment() {
    std::cout << "Cache associativity experiment" << std::endl;
    std::cout << "Ways(N)\t\t| Latency (ns/access)" << std::endl;
    std::cout << "----------------------------------------------------" << std::endl;

    for (int ways = 1; ways <= MAX_ASSOCIATIVITY_TO_TEST; ++ways) {
        const size_t buffer_size = ways * PROBE_STRIDE;
        char* buffer = new char[buffer_size];

        std::vector<Node*> nodes;
        for (int i = 0; i < ways; ++i) {
            nodes.push_back(reinterpret_cast<Node*>(buffer + i * PROBE_STRIDE));
        }

        std::vector<int> indices(ways);
        std::iota(indices.begin(), indices.end(), 0);
        std::random_shuffle(indices.begin(), indices.end());

        for (int i = 0; i < ways - 1; ++i) {
            nodes[indices[i]]->next = nodes[indices[i+1]];
        }
        nodes[indices[ways - 1]]->next = nodes[indices[0]];
        auto start_time = std::chrono::high_resolution_clock::now();
        const Node* current = nodes[0];
        for (long long i = 0; i < TOTAL_ACCESSES; ++i) {
            current = current->next;
        }
        auto end_time = std::chrono::high_resolution_clock::now();
        if (current == nullptr) {
            std::cout << "Impossible!" << std::endl;
        }
        auto duration_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time).count();
        double latency_per_access = static_cast<double>(duration_ns) / TOTAL_ACCESSES;
        std::cout << ways << "\t\t\t| " << std::fixed << std::setprecision(4) << latency_per_access << std::endl;
        delete[] buffer;
    }
}