#pragma once
#include "io/Instance.h"
#include "model/Solution.h"
#include <vector>
#include <utility>
#include "util/Metrics.h"
#include <unordered_set>
#include <cstdint>

using namespace std;
using Hash = uint64_t;
class GA {
public:
    GA(GAParams params, Instance instance);

    pair<vector<Solution>, GaRunMetrics> run(const Instance& inst);

private:
    GAParams params_;
    Instance instance_;
    vector<Solution> population;
    unordered_set<Hash> seen_hashes;

    void initializePopulation(bool use_smart_leader, float open_threshold);
    void nextGeneration(GaGeneration& current_metrics, bool use_local_search);
    void evaluatePopulation();
    vector<Solution> selectElite(int k) const;
    
    
    // Core Helpers
    void localSearch(vector<Solution>& childreen, GaGeneration& current_metrics, int needed);
    bool optimizeSolution(Solution& sol, int seed_offset); // <--- Updated Signature
    Solution generateGreedySolution();    // <--- NEW: Smart Initialization
    bool isDuplicate(Solution &sol);
};