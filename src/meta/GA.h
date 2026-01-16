#pragma once
#include "io/Instance.h"
#include "model/Solution.h"
#include <vector>
#include <utility>
#include "util/Metrics.h"

class GA {
public:
    GA(GAParams params, Instance instance);

    std::pair<std::vector<Solution>, GaRunMetrics> run(const Instance& inst, int elite_k_override = -1);

private:
    GAParams params_;
    Instance instance_;
    std::vector<Solution> population;

    void initializePopulation();
    void nextGeneration(GaGeneration& current_metrics);
    void evaluatePopulation();
    std::vector<Solution> selectElite(int k) const;
    
    
    // Core Helpers
    bool optimizeSolution(Solution& sol, int seed_offset); // <--- Updated Signature
    Solution generateGreedySolution();    // <--- NEW: Smart Initialization
};