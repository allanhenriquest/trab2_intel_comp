#include "controller/Solver.h"
#include "io/Writer.h"
#include "model/Evaluator.h"
#include "model/MonteCarlo.h"
#include <algorithm>
#include <iostream>
#include <filesystem>
#include <iomanip>
#include <sstream>
#include <numeric>
#include <fstream>

namespace fs = std::filesystem;
using namespace std;

// =========================================================
// PUBLIC METHODS
// =========================================================

PipelineResult Solver::solveInstance(const string& instance_path, const GAParams& ga_params, 
    const SAParams& sa_params, int mc_samples, int mc_k) 
{
    // Load instance (verbose=true to see parsing errors)
    Instance instance(instance_path, true);
    
    // SAFETY CHECK
    if (instance.n == 0) {
        cerr << "[CRITICAL ERROR] Instance failed to load (N=0). Aborting." << endl;
        PipelineResult fail;
        fail.stage1_ga = Solution(0, 0); 
        fail.stage2_det_sa = Solution(0, 0);
        fail.stage3_stoch_sa = Solution(0, 0);
        fail.stage3_avg_cost = 0.0;
        return fail; 
    }

    PipelineResult result;
    
    // --- STAGE 1: Deterministic GA ---
    // cout << "  [Stage 1] Running GA..." << endl;
    vector<Solution> ga_pool = solveDeterministic(instance, ga_params);
    
    if (ga_pool.empty()) {
        result.stage1_ga = Solution(instance.n, instance.m);
        result.stage2_det_sa = Solution(instance.n, instance.m);
        result.stage3_stoch_sa = Solution(instance.n, instance.m);
        result.stage3_avg_cost = 0.0;
        return result; 
    }
    result.stage1_ga = ga_pool.front(); 

    // --- STAGE 2: Deterministic SA ---
    // cout << "  [Stage 2] Running Deterministic SA..." << endl;
    vector<Solution> polished_pool = solveDeterministicSA(instance, sa_params, ga_pool);
    
    if (!polished_pool.empty()) {
        sort(polished_pool.begin(), polished_pool.end(), [](const Solution& a, const Solution& b){
            return a.total_cost < b.total_cost;
        });
        result.stage2_det_sa = polished_pool.front();
    } else {
        result.stage2_det_sa = result.stage1_ga; 
    }

    // --- STAGE 3: Stochastic SA (10 Runs) ---
    // cout << "  [Stage 3] Running Stochastic SA (10x)..." << endl;
    
    // fs::path p(instance.filePath);
    // string baseDir = "results/" + p.stem().string();
    // Writer::ensureDirectory(baseDir);
    // string logFile = baseDir + "/stochastic_log.csv";
    
    // // Create Log File with Header
    // {
    //     ofstream log(logFile);
    //     log << "Run,Cost" << endl;
    // }

    // vector<double> run_costs;
    // Solution best_of_all_runs;
    // best_of_all_runs.total_cost = numeric_limits<double>::infinity();

    // int n_runs = 10;
    // for (int i = 1; i <= n_runs; ++i) {
    //     vector<Solution> robust_pool = solveStochastic(instance, sa_params, polished_pool, result.stage2_det_sa, mc_samples, mc_k);

    //     double current_cost = 0.0;
    //     if (!robust_pool.empty()) {
    //         // Sort to find best in this pool
    //         sort(robust_pool.begin(), robust_pool.end(), [](const Solution& a, const Solution& b){
    //             return a.total_cost < b.total_cost;
    //         });
            
    //         Solution& best_run = robust_pool.front();
    //         current_cost = best_run.total_cost;

    //         // Update Global Best
    //         if (best_run.total_cost < best_of_all_runs.total_cost) {
    //             best_of_all_runs = best_run;
    //         }
    //     } else {
    //         // Fallback to deterministic if Stoch SA failed entirely
    //         current_cost = result.stage2_det_sa.total_cost;
    //     }

    //     run_costs.push_back(current_cost);
        
    //     // Log to file
    //     stringstream ss;
    //     ss << i << "," << fixed << setprecision(2) << current_cost;
    //     Writer::appendCSV(logFile, "", ss.str()); // Empty header since we created file above
    // }

    // // Finalize Stage 3 Results
    // if (best_of_all_runs.total_cost == numeric_limits<double>::infinity()) {
    //     result.stage3_stoch_sa = result.stage2_det_sa; // Fallback
    // } else {
    //     result.stage3_stoch_sa = best_of_all_runs;
    // }

    // // Calculate Average
    // double sum = accumulate(run_costs.begin(), run_costs.end(), 0.0);
    // result.stage3_avg_cost = (run_costs.empty()) ? 0.0 : (sum / run_costs.size());

    // // Save the Best Stochastic Solution to file
    // Writer::writeSolution(baseDir + "/solution_stage3_stoch_sa.txt", result.stage3_stoch_sa);

    return result;
}

void Solver::solveAllInDirectory(const string& dir_path, const GAParams& ga_params,
        const SAParams& sa_params, int mc_samples, int mc_k)
{
    vector<string> files;
    try {
        if (!fs::exists(dir_path)) { cerr << "Dir not found\n"; return; }
        for (const auto& entry : fs::directory_iterator(dir_path)) {
            if (entry.path().extension() == ".txt") files.push_back(entry.path().string());
        }
    } catch (...) { return; }
    
    sort(files.begin(), files.end());
    int total = files.size();
    
    cout << "Found " << total << " instances. Starting Pipeline..." << endl;

    ofstream summary("results/summary.csv");
    Writer::ensureDirectory("results");
    Writer::appendCSV("results/summary.csv", "Instance,GA,Det_SA,Stoch_SA", "");

    for (int i = 0; i < total; ++i) {
        string f = files[i];
        
        // Progress Bar
        float progress = (float)(i + 1) / total;
        int barWidth = 40;
        cout << "\r[";
        int pos = barWidth * progress;
        for (int b = 0; b < barWidth; ++b) {
            if (b < pos) cout << "=";
            else if (b == pos) cout << ">";
            else cout << " ";
        }
        cout << "] " << int(progress * 100.0) << "% " << fs::path(f).filename().string() << "   ";
        cout.flush();

        PipelineResult result = solveInstance(f, ga_params, sa_params, mc_samples, mc_k);
        stringstream row;
        row << fs::path(f).filename().string() << "," 
            << result.stage1_ga.total_cost << ","
            << result.stage2_det_sa.total_cost << ","
            << fixed << setprecision(2) << result.stage3_stoch_sa.total_cost;
        Writer::appendCSV("results/summary.csv", "", row.str());
    }
    cout << endl << "Batch run complete. Check 'results/' folders." << endl;
}

// =========================================================
// PIPELINE STAGES
// =========================================================

vector<Solution> Solver::solveDeterministic(const Instance& instance, const GAParams& ga_params) {
    GA ga(ga_params, instance);
    auto ga_res = ga.run(instance, ga_params.elite_k); 
    vector<Solution>& solutions = ga_res.first;
    RunMetrics& metrics = ga_res.second;

    fs::path p(metrics.instance_name);
    string baseDir = "results/" + p.stem().string();
    Writer::cleanUpDirectory(baseDir);
    Writer::ensureDirectory("results");
    Writer::ensureDirectory(baseDir);

    if (!solutions.empty()) {
        Writer::writeSolution(baseDir + "/solution_stage1_ga.txt", solutions.front());
    }
    
    for (const auto& gen : metrics.history) {
        stringstream row;
        row << gen.generation_index << "," << gen.best_cost << "," << gen.avg_cost << "," 
            << gen.time_total_ms << "," << gen.ls_improvements;
        Writer::appendCSV(baseDir + "/history_ga.csv", "Gen,Best,Avg,Time,Improv", row.str());
    }

    return solutions;
}

vector<Solution> Solver::solveDeterministicSA(const Instance& instance, const SAParams& sa_params, 
    const vector<Solution>& initialPool) 
{
    SA sa(sa_params);
    vector<Solution> refined_pool;

    CostEstimator det_estimator = [&](const Instance& inst, const Solution& sol) -> double {
        Solution temp = sol; 
        Evaluator::evaluateFull(inst, temp);
        return temp.total_cost;
    };

    for (const auto& seed : initialPool) {
        if (seed.openFacilities.empty()) continue; 
        refined_pool.push_back(sa.refine(instance, seed, det_estimator));
    }
    
    if (!refined_pool.empty()) {
        vector<Solution> sorted = refined_pool;
        sort(sorted.begin(), sorted.end(), [](const Solution& a, const Solution& b){
             return a.total_cost < b.total_cost; 
        });
        
        fs::path p(instance.filePath);
        string baseDir = "results/" + p.stem().string();
        Writer::writeSolution(baseDir + "/solution_stage2_det_sa.txt", sorted.front());
    }
    
    return refined_pool;
}

vector<Solution> Solver::solveStochastic(const Instance& instance, const SAParams& sa_params, 
    const vector<Solution>& initialPool, const Solution& best_deter_sol, int mc_samples, int mc_k) 
{
    SAParams stoch_params = sa_params; 
    stoch_params.iters_per_T = max(1, sa_params.iters_per_T / 5); 

    SA sa(stoch_params);
    vector<Solution> robust_pool;

    CostEstimator stoch_estimator = [&](const Instance& inst, const Solution& sol) -> double {
        return MonteCarlo::expectedCost(inst, sol, best_deter_sol, mc_samples, mc_k, 42);
    };

    for (const auto& seed : initialPool) {
        robust_pool.push_back(sa.refine(instance, seed, stoch_estimator));
    }
    // Note: Logging is now done in solveInstance loop
    return robust_pool;
}