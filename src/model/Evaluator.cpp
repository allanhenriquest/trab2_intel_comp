#include "Evaluator.h"
#include <limits>
#include <iostream>

using namespace std;

void Evaluator::evaluateFull(const Instance& inst, Solution& sol) {
    sol.total_cost = 0.0;
    sol.num_open_facilities = 0;
    
    // 1. Calculate Fixed Costs (Opening Costs)
    for (int i = 0; i < inst.n; ++i) {
        if (sol.openFacilities[i]) {
            // FIX: Use 'opening_costs' (plural) with index [i]
            sol.total_cost += inst.opening_costs[i];
            sol.num_open_facilities++;
        }
    }

    // 2. Calculate Allocation Costs & Update Cache
    for (int j = 0; j < inst.m; ++j) {
        double min_dist = numeric_limits<double>::infinity();
        int best_facility = -1;

        for (int i = 0; i < inst.n; ++i) {
            if (sol.openFacilities[i]) {
                double c = inst.allocation_costs[i][j];
                if (c < min_dist) {
                    min_dist = c;
                    best_facility = i;
                }
            }
        }
        
        // Update Optimization Cache: {FacilityIndex, Cost}
        sol.assigned_facility[j] = {best_facility, min_dist};
        
        // Add to total
        if (best_facility != -1) {
            sol.total_cost += min_dist;
        } else {
             // Heavy penalty if no facility serves this client
             sol.total_cost += 1e9; 
        }
    }
}

void Evaluator::openFacility(int facility, const Instance& inst, Solution& sol) {
    // 1. Add Opening Cost
    // FIX: Use 'opening_costs' (plural)
    sol.total_cost += inst.opening_costs[facility];
    sol.openFacilities[facility] = true;
    sol.num_open_facilities++;

    // 2. Update Clients (Incremental)
    // Check if this new facility is closer than their current assignment
    for (int j = 0; j < inst.m; ++j) {
        double current_cost = sol.assigned_facility[j].second;
        double new_cost = inst.allocation_costs[facility][j];

        if (new_cost < current_cost) {
            // Found a better facility! Update cost and cache.
            sol.total_cost -= current_cost; // Remove old cost
            sol.total_cost += new_cost;     // Add new cost
            
            sol.assigned_facility[j] = {facility, new_cost};
        }
    }
}

void Evaluator::closeFacility(int facility, const Instance& inst, Solution& sol) {
    // 1. Remove Opening Cost
    // FIX: Use 'opening_costs' (plural)
    sol.total_cost -= inst.opening_costs[facility];
    sol.openFacilities[facility] = false;
    sol.num_open_facilities--;

    // 2. Update Clients (Incremental)
    for (int j = 0; j < inst.m; ++j) {
        // Only re-evaluate clients who were served by the closed facility
        if (sol.assigned_facility[j].first == facility) {
            
            // Remove the old service cost from total
            sol.total_cost -= sol.assigned_facility[j].second;

            // Find the NEW best facility for this client
            double min_dist = numeric_limits<double>::infinity();
            int best_facility = -1;

            for (int i = 0; i < inst.n; ++i) {
                if (sol.openFacilities[i]) {
                    double c = inst.allocation_costs[i][j];
                    if (c < min_dist) {
                        min_dist = c;
                        best_facility = i;
                    }
                }
            }

            // Update Cache
            sol.assigned_facility[j] = {best_facility, min_dist};
            
            // Add new service cost
            if (best_facility != -1) {
                sol.total_cost += min_dist;
            } else {
                sol.total_cost += 1e9; // Penalty
            }
        }
    }
}