#pragma once
#include <vector>
#include <limits>
#include <utility> // For pair

using namespace std;

struct Solution {
    // N: Number of Facilities
    // M: Number of Clients
    
    vector<bool> openFacilities; // Size N
    
    vector<pair<int, double>> assigned_facility; // Size M: (facility_index, allocation_cost)

    double total_cost;

    // Constructors
    Solution();
    Solution(int n, int m); // UPDATED: Takes both N and M

    // Helpers
    void ensureAtLeastOneOpen();
};