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
    string filePath;
    int n;
    double opening_cost;       
    vector<vector<double>> allocation_costs; 
    
    Instance(const string filePath, bool verbose = false);
};
