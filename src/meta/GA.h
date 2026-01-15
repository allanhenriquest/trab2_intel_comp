#pragma once
#include "io/Instance.h"
#include "model/Solution.h"
#include <vector>
#include <utility>

struct GAParams {
    int pop_size{100};
    int max_generations{300};
    int elite_k{10};
    double mutation_rate{0.5}; // Increased slightly for better exploration
    unsigned long long seed{42};
};

struct RunMetrics {
    std::string instance_name;
    int n_facilities;
    double final_cost;
    long long total_time_ms;
    
    struct Generation {
        int generation_index;
        double best_cost;
        double avg_cost;
        long long time_total_ms;
        long long time_evolution_ms;
        long long time_localsearch_ms;
        int ls_improvements;
    };
    std::vector<Generation> history;
};

class GA {
public:
    GA(GAParams params, Instance instance);

    std::pair<std::vector<Solution>, RunMetrics> run(const Instance& inst, int elite_k_override = -1);

private:
    GAParams params_;
    Instance instance_;
    std::vector<Solution> population;

    void initializePopulation();
    void nextGeneration(RunMetrics::Generation& current_metrics);
    void evaluatePopulation();
    std::vector<Solution> selectElite(int k) const;
    
    
    // Core Helpers
    bool optimizeSolution(Solution& sol, int seed_offset); // <--- Updated Signature
    Solution generateGreedySolution();    // <--- NEW: Smart Initialization
};