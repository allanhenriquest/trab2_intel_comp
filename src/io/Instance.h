#pragma once
#include <string>
#include <vector>

using namespace std;

class Instance {
public:
    int n; // Facilities
    int m; // Customers
    
    // Arrays to match the problem data
    vector<long> opening_costs;          // Cost to open facility i
    vector<vector<long>> allocation_costs; // Cost to serve client j from facility i
    
    string filePath;

    Instance(const string& path, bool verbose = false);
};