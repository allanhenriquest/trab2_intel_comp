#pragma once
#include "io/Instance.h"
#include "model/Solution.h"

#pragma once
#include "io/Instance.h"
#include "model/Solution.h"

class MonteCarlo {
public:
    // Calculates expected cost under stochastic service costs (Log-Normal).
    // Uses 'best_deter_sol' to define the penalty threshold (Peidro et al., 2024).
    static double expectedCost(const Instance& inst, const Solution& sol, 
                               const Solution& best_deter_sol, 
                               int samples, int k, unsigned long long seed);
};