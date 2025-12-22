#include "Solution.h"

using namespace std;

void Solution::resize(int n) {
    y.assign(n, false);
    assigned_facility.assign(n, -1);
    current_cost_per_client.assign(n, 0.0);
    total_cost = 0.0;
}

void Solution::ensureAtLeastOneOpen() {
    bool any = false;
    for (bool b : y) { if (b) { any = true; break; } }
    if (!any && !y.empty()) y[0] = true;
}

