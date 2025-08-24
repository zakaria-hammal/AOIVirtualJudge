#include <bits/stdc++.h>
using namespace std;

int main() {
    vector<vector<int>> memory_hog;

    // This will trigger MLE by allocating increasingly larger chunks
    try {
        long long size = 1000000; // Start with 1 million elements
        while (true) {
            // Double the allocation each iteration
            vector<int> chunk(size);
            memory_hog.push_back(chunk);
            size *= 2;
        }
    } catch (const bad_alloc& e) {
        // Even if allocation fails, keep trying smaller allocations
        while (true) {
            vector<int> small_chunk(1000);
            memory_hog.push_back(small_chunk);
        }
    }

    return 0;
}
