#pragma once
#include "model/Solution.h"
#include "io/Instance.h"
#include <functional>
#include <vector>
#include "util/Metrics.h" 

using namespace std;

// Alteração: O estimador agora recebe 'samples' dinamicamente
using CostEstimator = function<double(const Instance&, const Solution&, int samples)>;

class SA {
public:
    explicit SA(SAParams params);

    pair<Solution, vector<SaStep>> refine(const Instance& inst, const Solution& initial_sol, const CostEstimator& estimator);

private:
    SAParams params_;
};