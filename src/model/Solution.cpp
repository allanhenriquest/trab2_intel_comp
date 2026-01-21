#include "Solution.h"
#include <algorithm>
#include <random>

Solution::Solution() : total_cost(0), expected_cost(0.0), 
                        num_open_facilities(0), hash(0) {}

Solution::Solution(int n, int m) : total_cost(0.0), expected_cost(0.0), 
                                    num_open_facilities(0), hash(0) {
    openFacilities.assign(n, false);
    assigned_facility.assign(m, {-1, numeric_limits<long>::infinity()});
}

void Solution::ensureAtLeastOneOpen() {
    if(num_open_facilities > 0) 
        return;
    openFacilities[0] = true;
    num_open_facilities = 1;
}

void Solution::computeHash()
{
    static constexpr Hash P = 1315423911ULL;
    Hash h = 0;

    for (bool b : openFacilities) {          // ok: conversão implícita do proxy
        h += (h << 1) ^ (Hash(b) * P);
    }
    hash = h;
}