#include "Evaluator.h"
#include <limits>

using namespace std;

void Evaluator::evaluateFull(const Instance& inst, Solution& sol) const {
    sol.total_cost = 0.0;
    // Fixed costs
    for (int i = 0; i < inst.n; ++i) if (sol.y.size() > (size_t)i && sol.y[i]) sol.total_cost += inst.opening_costs[i];
    // Assign each client to best open facility
    sol.assigned_facility.assign(inst.n, -1);
    sol.current_cost_per_client.assign(inst.n, numeric_limits<double>::infinity());
    for (int j = 0; j < inst.n; ++j) {
        for (int i = 0; i < inst.n; ++i) if (sol.y[i]) {
            double c = inst.allocation_costs[i][j];
            if (c < sol.current_cost_per_client[j]) {
                sol.current_cost_per_client[j] = c;
                sol.assigned_facility[j] = i;
            }
        }
        if (sol.assigned_facility[j] >= 0) sol.total_cost += sol.current_cost_per_client[j];
    }
}

void Evaluator::openFacility(int k, const Instance& inst, Solution& sol) const {
    if (!sol.y[k]) {
        sol.y[k] = true;
        sol.total_cost += inst.opening_costs[k];
        for (int j = 0; j < inst.n; ++j) {
            double c = inst.allocation_costs[k][j];
            if (c < sol.current_cost_per_client[j]) {
                if (sol.assigned_facility[j] >= 0) sol.total_cost -= sol.current_cost_per_client[j];
                sol.current_cost_per_client[j] = c;
                sol.assigned_facility[j] = k;
                sol.total_cost += c;
            }
        }
    }
}

void Evaluator::closeFacility(int k, const Instance& inst, Solution& sol) const {
    if (sol.y[k]) {
        sol.y[k] = false;
        sol.total_cost -= inst.opening_costs[k];
        for (int j = 0; j < inst.n; ++j) {
            if (sol.assigned_facility[j] == k) {
                // remove current contribution and find best among remaining open facilities
                sol.total_cost -= sol.current_cost_per_client[j];
                sol.current_cost_per_client[j] = numeric_limits<double>::infinity();
                sol.assigned_facility[j] = -1;
                for (int i = 0; i < inst.n; ++i) if (sol.y[i]) {
                    double c = inst.allocation_costs[i][j];
                    if (c < sol.current_cost_per_client[j]) {
                        sol.current_cost_per_client[j] = c;
                        sol.assigned_facility[j] = i;
                    }
                }
                if (sol.assigned_facility[j] >= 0) sol.total_cost += sol.current_cost_per_client[j];
            }
        }
    }
}
