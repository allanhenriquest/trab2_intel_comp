#pragma once
#include "model/Solution.h"
#include "io/Instance.h"
#include <functional>
#include <vector>
#include "util/Metrics.h" 

using namespace std;

using CostEstimator = function<double(const Instance&, const Solution&)>;

class SA {
public:
    explicit SA(SAParams params);

    // Agora retorna a Solução Refinada E o Histórico da execução
    pair<Solution, vector<SaStep>> refine(const Instance& inst, const Solution& initial_sol, const CostEstimator& estimator);

private:
    SAParams params_;
};