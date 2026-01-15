#pragma once
#include "io/Instance.h"
#include "model/Solution.h"

class MonteCarlo {
public:
    // Calculates the expected cost of a solution under stochastic demand.
    // best_deter_sol is passed for potential reference/caching strategies (optional usage).
    static double expectedCost(const Instance& inst, const Solution& sol, 
                               const Solution& best_deter_sol, 
                               int samples, int k, unsigned long long seed);
};