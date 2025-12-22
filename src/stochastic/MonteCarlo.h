#pragma once

#include "core/Instance.h"
#include "model/Solution.h"

using namespace std;

// Monte Carlo cost estimator for stochastic evaluation.
// Provides methods to estimate expected cost (and penalties, if any).
class MonteCarlo {
public:
    // Estimate the expected total cost via 'samples' random scenarios.
    // Note: scenario generation details TBD; this is a placeholder signature.
    double expectedCost(const Instance& inst, const Solution& sol, int samples) const;
};
