#include "Evaluator.h"
#include <limits>

using namespace std;

void Evaluator::evaluateFull(const Instance& inst, Solution& sol) {    
    // Fixed costs
    for (int i = 0; i < inst.n; ++i) 
        if (sol.openFacilities.size() > (size_t)i && sol.openFacilities[i]) 
            sol.total_cost += inst.opening_cost;

    for (int j = 0; j < inst.n; ++j) {
        for (int i = 0; i < inst.n; ++i) if (sol.openFacilities[i]) {
            double c = inst.allocation_costs[i][j];
            if (c < sol.assigned_facility[j].second) {
                sol.assigned_facility[j].second = c;
                sol.assigned_facility[j].first = i;
            }
        }
        if (sol.assigned_facility[j].first >= 0) sol.total_cost += sol.assigned_facility[j].second;
    }
}

void Evaluator::openFacility(int k, const Instance& inst, Solution& sol) {
    if (!sol.openFacilities[k]) {
        sol.openFacilities[k] = true;
        sol.total_cost += inst.opening_cost;
        for (int j = 0; j < inst.n; ++j) {
            double c = inst.allocation_costs[k][j];
            if (c < sol.assigned_facility[j].second) {
                if (sol.assigned_facility[j].first >= 0) sol.total_cost -= sol.assigned_facility[j].second;
                sol.assigned_facility[j].second = c;
                sol.assigned_facility[j].first = k;
                sol.total_cost += c;
            }
        }
    }
}

void Evaluator::closeFacility(int k, const Instance& inst, Solution& sol) {
    if (sol.openFacilities[k]) {
        sol.openFacilities[k] = false;
        sol.total_cost -= inst.opening_cost;
        for (int j = 0; j < inst.n; ++j) {
            if (sol.assigned_facility[j].first == k) {
                // remove current contribution and find best among remaining open facilities
                sol.total_cost -= sol.assigned_facility[j].second;
                sol.assigned_facility[j].second = numeric_limits<double>::infinity();
                sol.assigned_facility[j].first = -1;
                for (int i = 0; i < inst.n; ++i) if (sol.openFacilities[i]) {
                    double c = inst.allocation_costs[i][j];
                    if (c < sol.assigned_facility[j].second) {
                        sol.assigned_facility[j].second = c;
                        sol.assigned_facility[j].first = i;
                    }
                }
                if (sol.assigned_facility[j].first >= 0) sol.total_cost += sol.assigned_facility[j].second;
            }
        }
    }
}
