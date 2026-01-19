#pragma once

#include "io/Instance.h"
#include "model/Solution.h"

using namespace std;

// Deterministic evaluator with full and incremental update methods.
class Evaluator {
public:
    // Compute total cost from scratch; fills assignments and costs.
    static void evaluateFull(const Instance& inst, Solution& sol);

    // Open facility k; update assignments and total cost in O(n).
    static void openFacility(int k, const Instance& inst, Solution& sol);

    // Close facility k; reassign affected clients and update total cost in O(n).
    static void closeFacility(int k, const Instance& inst, Solution& sol);

    static long delta(int facility, const Instance& inst, const Solution& sol);
};
