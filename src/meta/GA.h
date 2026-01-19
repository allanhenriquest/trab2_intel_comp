#pragma once
#include "io/Instance.h"
#include "model/Solution.h"
#include <vector>
#include <utility>
#include "util/Metrics.h"
#include <unordered_set>
#include <cstdint>
#include <unordered_map>

using namespace std;

// Type alias for solution hash (64-bit unsigned integer)
using Hash = uint64_t;

class GA {
public:
    // Constructor: Initializes GA with parameters and problem instance
    GA(GAParams params, Instance instance);

    // Main execution method: Returns the elite population and run metrics
    pair<vector<Solution>, GaRunMetrics> run(const Instance& inst);

private:
    GAParams params_;
    Instance instance_;
    vector<Solution> population;
    
    // Hash map to track duplicate solutions (Hash -> Occurrence Count)
    unordered_map<Hash, int> seen_hashes;
    
    // Counter for duplicates found in the current generation
    long long gen_duplicates{0};

    // --- Core Evolutionary Methods ---

    // Initializes population with random solutions and optionally injects a "Smart Leader"
    void initializePopulation(bool use_smart_leader, float open_threshold);

    // Evolves the population by one generation (Selection -> Crossover -> Mutation -> Replacement)
    // 'current_mutation_rate' is passed dynamically to allow adaptive control
    void nextGeneration(GaGeneration& current_metrics, bool use_local_search, double current_mutation_rate);

    // Evaluates the fitness (cost) of all individuals in the population
    void evaluatePopulation();

    // Selects the top 'k' best solutions (Elitism)
    vector<Solution> selectElite(int k) const;
    
    
    // --- Helper Methods ---

    // Applies Local Search to improved children (Hill Climbing)
    void localSearch(vector<Solution>& childreen, GaGeneration& current_metrics, int needed);

    // Single-solution optimization routine. Returns true if improvement was found.
    // 'seed_offset' ensures thread-safe deterministic randomness.
    bool optimizeSolution(Solution& sol, int seed_offset); 

    // Generates a high-quality initial solution using a constructive greedy heuristic
    Solution generateGreedySolution();    

    // Checks if a solution is a duplicate based on its hash.
    // 'insert_if_new': If true, adds the hash to the tracking set if not present.
    bool isDuplicate(Solution &sol, bool insert_if_new=true);
};