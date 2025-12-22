#pragma once

#include <vector>

using namespace std;

// Represents a solution to UFLP.
// Attributes:
// - y: binary vector of open facilities (size n)
// - assigned_facility: facility index serving each client (size n)
// - current_cost_per_client: serving cost for each client under current assignment (size n)
// - total_cost: total objective value (fixed + serving)
struct Solution {
    vector<bool> y;
    vector<int> assigned_facility; // size n
    vector<double> current_cost_per_client; // size n
    double total_cost{0.0};

    // Resize internal vectors for a given n; sets all closed/unassigned, zero costs.
    void resize(int n);

    // Ensure at least one facility is open; if none, open a default one (e.g., 0).
    void ensureAtLeastOneOpen();
};
