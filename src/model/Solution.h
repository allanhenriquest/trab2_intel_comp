#pragma once

#include <vector>

using namespace std;

// Represents a solution to UFLP.
// Attributes:
// - y: binary vector of open facilities (size n)
// - assigned_facility: for each client j, the assigned facility i and its cost (size n)
// - total_cost: total objective value (fixed + serving)
struct Solution {
    vector<bool> openFacilities; // size n
    vector<pair<int, double>> assigned_facility; // size n
    double total_cost;
    unsigned long long seed{42};

    Solution(int n);

    // Ensure at least one facility is open; if none, open a default one (e.g., 0).
    void ensureAtLeastOneOpen();
};
