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
    Solution stage1_ga;       // Best found by Genetic Algorithm
    Solution stage2_det_sa;   // Best after Deterministic Polishing
    
    // Stage 3 Results
    Solution stage3_stoch_sa; // The single best solution found across all runs
    double stage3_avg_cost;   // The average cost of the solutions found in the 10 runs
};

class Solver
{
public:
    static PipelineResult solveInstance(const string& instance_path, const GAParams& ga_params,
        const SAParams& sa_params, int mc_samples, int mc_k);

    static void solveAllInDirectory(const string& dir_path, const GAParams& ga_params,
        const SAParams& sa_params, int mc_samples, int mc_k);
    
private:
    static vector<Solution> solveDeterministic(const Instance& instance, const GAParams& ga_params);
    static vector<Solution> solveDeterministicSA(const Instance& instance, const SAParams& sa_params, const vector<Solution>& initialPool);
    static vector<Solution> solveStochastic(const Instance& instance, const SAParams& sa_params, const vector<Solution>& initialPool, const Solution& best_deter_sol, int mc_samples, int mc_k);
};