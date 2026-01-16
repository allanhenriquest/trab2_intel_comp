#pragma once
#include "model/Solution.h"
#include "io/Instance.h"
#include <functional>
#include <vector>
#include "util/Metrics.h"

using namespace std;

// Flexible function type to calculate cost (Deterministic or Stochastic)
using CostEstimator = function<double(const Instance&, const Solution&)>;

class SA {
public:
    explicit SA(SAParams params);
    Solution refine(const Instance& inst, const Solution& seed, const CostEstimator& estimator);

private:
    SAParams params_;
};