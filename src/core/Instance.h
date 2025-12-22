#pragma once

#include <vector>
#include <string>

using namespace std;

// Holds the internal representation of a UFLP instance.
// Attributes:
// - n: number of facilities and clients (n = m)
// - opening_costs: size n, fixed cost to open each facility
// - allocation_costs: n x n matrix of serving costs
class Instance {
public:
    int n{0};
    vector<double> opening_costs;           
    vector<vector<double>> allocation_costs; 

    // Construct an empty instance.
    Instance() = default;

    // Construct with size; opening costs set to 0, allocation costs to 0.
    Instance(int n_);

    // Validate basic consistency (square matrix n x n). Throws on error.
    void validate() const;
};
