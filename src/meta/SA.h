#pragma once
#include "model/Solution.h"
#include "io/Instance.h"
#include <functional>
#include <vector>
#include "util/Metrics.h" // Assuming SAParams is defined here

using namespace std;

// Alias for cost function: can be deterministic (Evaluator) or stochastic (MonteCarlo)
using CostEstimator = function<double(const Instance&, const Solution&)>;

class SA {
public:
    explicit SA(SAParams params);

    // Runs the Simulated Annealing process.
    // 'seed' is the starting solution (usually from GA).
    // 'estimator' is the cost function (in Phase 2, this will be the MonteCarlo simulation).
    Solution refine(const Instance& inst, const Solution& initial_sol, const CostEstimator& estimator);

private:
    SAParams params_;
};