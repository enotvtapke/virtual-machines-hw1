#include <iostream>
#include <vector>
#include <chrono>
#include <numeric>
#include <algorithm>
#include <cstring>
#include <iomanip> // For std::fixed and std::setprecision

// --- Configuration ---

// We will test for associativity up to this number. 32 is a safe upper bound for most CPUs.
constexpr int MAX_ASSOCIATIVITY_TO_TEST = 16;

// The stride between memory accesses. To ensure addresses compete for the same cache set
// in the L1 cache, we use a stride that is a large power of two.
// 64 KB is a common choice as it's typically larger than an L1 data cache slice,
// making it very likely that `addr` and `addr + 64KB` map to the same L1 set.
constexpr size_t PROBE_STRIDE = 64 * 1024;

// The total number of pointer-chasing memory accesses to perform for each timing run.
// A large number is needed to get a stable average.
constexpr long long TOTAL_ACCESSES = 100 * 1000 * 1000;

// A simple struct for our linked list nodes. The 'next' pointer is what we will chase.
// The padding is to prevent the CPU's adjacent-line prefetcher from interfering.
struct Node {
    Node* next;
    // char padding[64 - sizeof(Node*)]; // Pad to 64 bytes
};

int main() {

    std::cout << "Starting L1 cache associativity experiment..." << std::endl;
    std::cout << "Ways\t| ns/access" << std::endl;
    std::cout << "----------------------------------------------------" << std::endl;

    // We will iterate from N=1 way up to the max we want to test.
    for (int ways = 1; ways <= MAX_ASSOCIATIVITY_TO_TEST; ++ways) {

        // --- 1. Setup the memory region ---
        // Allocate a large enough memory buffer to hold 'ways' number of nodes,
        // each separated by our large PROBE_STRIDE.
        size_t buffer_size = ways * PROBE_STRIDE;
        char* buffer = new char[buffer_size];

        // Create the nodes at intervals of PROBE_STRIDE within the buffer.
        std::vector<Node*> nodes;
        for (int i = 0; i < ways; ++i) {
            nodes.push_back(reinterpret_cast<Node*>(buffer + i * PROBE_STRIDE));
        }

        std::vector<int> indices(ways);
        std::iota(indices.begin(), indices.end(), 0); // Fill with 0, 1, 2...
        std::random_shuffle(indices.begin(), indices.end()); // Shuffle them

        // --- 2. Create the pointer-chasing cycle ---
        // Link the nodes together in a pseudo-random order to form a cycle.
        // This is a simple permutation: 0 -> 1 -> 2 -> ... -> (N-1) -> 0
        for (int i = 0; i < ways - 1; ++i) {
            nodes[indices[i]]->next = nodes[indices[i+1]];
        }
        // Link the last node back to the first to close the cycle.
        nodes[indices[ways - 1]]->next = nodes[indices[0]];

        // --- 3. Run the benchmark ---
        auto start_time = std::chrono::high_resolution_clock::now();

        // Start chasing pointers from the first node.
        Node* current = nodes[0];
        for (long long i = 0; i < TOTAL_ACCESSES; ++i) {
            current = current->next;
        }

        auto end_time = std::chrono::high_resolution_clock::now();

        // This is just to ensure the compiler doesn't optimize away the loop.
        if (current == nullptr) {
            std::cout << "Impossible!" << std::endl;
        }

        // --- 4. Calculate and print results ---
        auto duration_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time).count();
        double latency_per_access = static_cast<double>(duration_ns) / TOTAL_ACCESSES;

        std::cout << ways << "\t\t| " << std::fixed << std::setprecision(4) << latency_per_access << std::endl;

        // --- 5. Cleanup ---
        delete[] buffer;
    }

    std::cout << "----------------------------------------------------" << std::endl;
    std::cout << "Experiment finished." << std::endl;

    return 0;
}