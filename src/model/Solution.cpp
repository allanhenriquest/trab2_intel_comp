#include "Solution.h"
#include <algorithm>
#include <random>

using namespace std;

Solution::Solution() : total_cost(0), expected_cost(0.0), num_open_facilities(0) {}

// UPDATED: Now takes n (facilities) and m (clients)
Solution::Solution(int n, int m) : total_cost(0.0), num_open_facilities(0) {
    openFacilities.assign(n, false);
    
    assigned_facility.assign(m, {-1, numeric_limits<double>::infinity()});
}

void Solution::ensureAtLeastOneOpen() {
    if(num_open_facilities > 0) 
        return;
    openFacilities[0] = true;
    num_open_facilities = 1;
}