#pragma once
#include <vector>
#include <limits>
#include <utility> // For pair

using namespace std;

struct Solution {
    // N: Number of Facilities
    // M: Number of Clients
    
    vector<bool> openFacilities; // Size N
    
    // Cache: {FacilityIndex, ServiceCost} for each client
    // Size M (Must be M, not N!)
    vector<pair<int, double>> assigned_facility; 

    double total_cost;

    // Constructors
    Solution();
    Solution(int n, int m); // UPDATED: Takes both N and M

    // Helpers
    void ensureAtLeastOneOpen();
};