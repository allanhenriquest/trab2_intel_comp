#pragma once

#include "core/Instance.h"
#include "model/Solution.h"

using namespace std;

// Deterministic evaluator with full and incremental update methods.
class Evaluator {
public:
    // Compute total cost from scratch; fills assignments and costs.
    void evaluateFull(const Instance& inst, Solution& sol) const;

    // Open facility k; update assignments and total cost in O(n).
    void openFacility(int k, const Instance& inst, Solution& sol) const;

    // Close facility k; reassign affected clients and update total cost in O(n).
    void closeFacility(int k, const Instance& inst, Solution& sol) const;
};
