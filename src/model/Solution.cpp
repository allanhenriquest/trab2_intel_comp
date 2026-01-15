#include "Solution.h"
#include <algorithm>
#include <random>

using namespace std;

Solution::Solution() : total_cost(0.0) {}

// UPDATED: Now takes n (facilities) and m (clients)
Solution::Solution(int n, int m) : total_cost(0.0) {
    // 1. Setup Facilities (Size N)
    openFacilities.assign(n, false);
    
    // 2. Setup Client Cache (Size M)
    // -1 means "no facility assigned"
    // infinity means "infinite cost"
    assigned_facility.assign(m, {-1, numeric_limits<double>::infinity()});
}

void Solution::ensureAtLeastOneOpen() {
    for (bool open : openFacilities) {
        if (open) return;
    }
    // If none open, open the first one
    if (!openFacilities.empty()) {
        openFacilities[0] = true;
    }
}