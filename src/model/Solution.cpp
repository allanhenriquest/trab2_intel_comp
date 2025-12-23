#include "Solution.h"
#include <limits>

using namespace std;

Solution::Solution(int n) {
    openFacilities.assign(n, false);
    assigned_facility.assign(n, {-1, numeric_limits<double>::infinity()});
    total_cost = 0.0;
}

void Solution::ensureAtLeastOneOpen() {
    bool any = false;
    for (bool b : openFacilities) { if (b) { any = true; break; } }
    if (!any && !openFacilities.empty()) openFacilities[0] = true;
}

