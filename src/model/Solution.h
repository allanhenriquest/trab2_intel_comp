#pragma once
#include <vector>
#include <limits>
#include <utility> // For pair

using namespace std;

struct Solution {
    // N: Number of Facilities
    // M: Number of Clients
    
    vector<bool> openFacilities; // Size N
    vector<pair<int, long>> assigned_facility; // Size M: (facility_index, allocation_cost)

    long total_cost;
    int num_open_facilities; // Cached count of open facilities

    // Constructors
    Solution();
    Solution(int n, int m); // UPDATED: Takes both N and M

    // Helpers
    void ensureAtLeastOneOpen();
};