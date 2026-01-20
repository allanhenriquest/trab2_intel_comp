#pragma once

#include "model/Solution.h"
#include "io/Instance.h"
#include "meta/GA.h"
#include "meta/SA.h"
#include <vector>
#include <string>
#include <map>

using namespace std;

// Container for the results of all 3 stages (Full Pipeline)
struct PipelineResult {
    Solution OBD; // Best Deterministic solution (from GA)
    double OBD_t; // Time taken by GA stage (ms)
    
    double OBD_S; // Expected Cost of OBD (via Monte Carlo Simulation)
    
    Solution OBS; // Best Stochastic solution (from SA)
    double OAS;   // Average Expected Cost of the robust pool
    double OBS_t; // Time taken by SA stage (ms)
};

// Known optimal values for MED instances (for Gap calculation)
const map<string, long> BEST = {
    {"500-10", 798577},
    {"500-100", 326790},
    {"500-1000", 99169},
    {"1000-10", 1434154},
    {"1000-100", 607878},
    {"1000-1000", 220560},
    {"1500-10", 2000801},
    {"1500-100", 866454},
    {"1500-1000", 334962},
    {"2000-10", 2558118},
    {"2000-100", 1122748},
    {"2000-1000", 437686},
    {"2500-10", 3101800},
    {"2500-100", 1347516},
    {"2500-1000", 534405},
    {"3000-10", 3570766},
    {"3000-100", 1606969},
    {"3000-1000", 643463}
};

class Solver
{
public:
    // Solves a single instance running the full pipeline: GA -> Monte Carlo -> SA
    static PipelineResult solveInstance(const string& instance_path, const GAParams& ga_params,
        const SAParams& sa_params);

    // Batch processes an entire directory, generating summary CSVs and charts
    static void solveAllInDirectory(const string& dir_path, const GAParams& ga_params,
        const SAParams& sa_params);
    
    void runStatisticalAnalysis(const std::string &dir_path, const GAParams &ga_params, const SAParams &sa_params, int runs);
private:
    // Helper: Runs the GA (Deterministic Stage)
    static pair<vector<Solution>, GaRunMetrics> solveDeterministic(const Instance& instance, const GAParams& ga_params);
    
    // Helper: Runs the SA (Stochastic Stage) starting from GA solutions
    static vector<Solution> solveStochastic(const Instance& instance, 
        const SAParams& sa_params, const vector<Solution>& initialPool, const Solution& best_deter_sol);
};