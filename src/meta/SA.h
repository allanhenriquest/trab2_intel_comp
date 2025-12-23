#pragma once

#include <functional>
#include "io/Instance.h"
#include "model/Solution.h"

using namespace std;
using CostEstimator = function<double(const Instance&, const Solution&)>;

struct SAParams {
    double T0{1.0};
    double alpha{0.95};
    int iters_per_T{1000};
    double Tmin{1e-3};
    unsigned long long seed{123};
};

// Simulated Annealing for the stochastic refinement phase.
class SA {
public:
    explicit SA(SAParams params);

    // Refine a given seed solution using the provided cost estimator (e.g., Monte Carlo).
    Solution refine(const Instance& inst, const Solution& seed, const CostEstimator& estimator);

private:
    SAParams params_;
};
