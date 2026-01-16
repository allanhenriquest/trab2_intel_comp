#pragma once

#include "model/Solution.h"
#include "io/Instance.h"
#include "meta/GA.h"
#include "meta/SA.h"
#include <vector>
#include <string>

using namespace std;

// Container for the results of all 3 stages
struct PipelineResult {
    Solution best_ga;       // Best found by Genetic Algorithm
    double ga_time_ms; // Time taken by GA stage
    Solution best_stoch_sa; // The single best solution found across all runs
    double stoch_sa_avg_cost;   // The average cost of the solutions found in the 10 runs
    double stoch_sa_time_ms; // Time taken by Stochastic SA stage
};

class Solver
{
public:
    static PipelineResult solveInstance(const string& instance_path, const GAParams& ga_params,
        const SAParams& sa_params, int mc_samples, int mc_k);

    static void solveAllInDirectory(const string& dir_path, const GAParams& ga_params,
        const SAParams& sa_params, int mc_samples, int mc_k);
    
private:
    static pair<vector<Solution>, GaRunMetrics> solveDeterministic(const Instance& instance, const GAParams& ga_params);
    static vector<Solution> solveStochastic(const Instance& instance, const SAParams& sa_params, const vector<Solution>& initialPool, const Solution& best_deter_sol, int mc_samples, int mc_k);
};