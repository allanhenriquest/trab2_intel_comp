#pragma once
#include "model/Solution.h"
#include "io/Instance.h"
#include <functional>
#include <vector>

using namespace std;

struct SAParams {
    double T0{1000.0};
    double Tmin{0.01};
    double alpha{0.95};
    int iters_per_T{100};
    unsigned long long seed{42};
};

// Flexible function type to calculate cost (Deterministic or Stochastic)
using CostEstimator = function<double(const Instance&, const Solution&)>;

class SA {
public:
    explicit SA(SAParams params);
    Solution refine(const Instance& inst, const Solution& seed, const CostEstimator& estimator);

private:
    SAParams params_;
};