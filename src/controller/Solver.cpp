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
#include "util/Metrics.h"
#include "util/Timer.h"

namespace fs = std::filesystem;
using namespace std;

// List of files for Test Mode (Hardcoded for convenience)
const vector<string> test_files = {
    "./instancias_MED/500-10.txt", "./instancias_MED/500-100.txt", "./instancias_MED/500-1000.txt",
    "./instancias_MED/1500-10.txt", "./instancias_MED/1500-100.txt", "./instancias_MED/1500-1000.txt",
    "./instancias_MED/2500-10.txt", "./instancias_MED/2500-100.txt", "./instancias_MED/2500-1000.txt"};

PipelineResult Solver::solveInstance(const string &instance_path, const GAParams &ga_params,
                                     const SAParams &sa_params)
{
    // 1. Load Instance
    // verbose=true prints errors if parsing fails
    Instance instance(instance_path, true);
    
    // Extract naming info for file structure and output paths
    fs::path p(instance.filePath);
    string instance_name = p.stem().string();
    string baseDir = "results/" + instance_name;

    // Safety Check: Abort if instance is empty/invalid
    if (instance.n == 0)
    {
        cerr << "[CRITICAL ERROR] Instance failed to load (N=0). Aborting." << endl;
        PipelineResult fail;
        fail.OBD = Solution(0, 0);
        fail.OBS = Solution(0, 0);
        fail.OAS = 0.0;
        return fail;
    }

    PipelineResult result;

    // ====================================================
    // STAGE 1: Deterministic GA (The "Robust" Solver)
    // ====================================================
    // Runs the Genetic Algorithm to find the Best Deterministic Solution (OBD)
    auto ga_res = solveDeterministic(instance, ga_params);
    vector<Solution> &ga_pool = ga_res.first;
    GaRunMetrics &ga_metrics = ga_res.second;

    result.OBD_t = ga_metrics.total_time_ms;

    if (ga_pool.empty())
    {
        // Fallback for catastrophic failure (should not happen with robust GA)
        result.OBD = Solution(instance.n, instance.m);
        result.OBS = Solution(instance.n, instance.m);
        result.OAS = 0.0;
        return result;
    }

    // Set OBD (Best Deterministic) - Elite[0] is guaranteed to be best due to sorting in GA
    result.OBD = ga_pool.front();
    
    // Evaluate OBD under Uncertainty (Simulation)
    // We use a HIGH number of samples (100,000) for accurate reporting of the OBD_S
    result.OBD.expected_cost = MonteCarlo::expectedCost(instance, result.OBD, result.OBD, 
                        100000, sa_params.mc_k, sa_params.seed);
    result.OBD_S = result.OBD.expected_cost;

    // Save Stage 1 Result
    if (!ga_pool.empty())
    {
        Writer::writeSolution(baseDir + "/solution_stage1_ga.txt", result.OBD, ga_params.seed);
    }

    // ====================================================
    // STAGE 2: Stochastic SA (Refinement)
    // ====================================================

    if (!sa_params.solve)
    {
        // If SA is disabled via flags, fill with dummy/deterministic values
        result.OBS = result.OBD; 
        result.OAS = result.OBD_S;
        result.OBS_t = 0.0;
    }
    else
    {
        Writer::ensureDirectory(baseDir);
        Timer sa_timer;
        sa_timer.start();
        
        // Run SA starting from the GA pool.
        // NOTE: The SA inside uses a LOW sample count (e.g., 100) for performance.
        vector<Solution> robust_pool = solveStochastic(instance, sa_params, ga_pool, result.OBD);
        
        sa_timer.stop();
        result.OBS_t = sa_timer.elapsedMs();

        // CRITICAL STEP: Re-evaluate the robust pool with HIGH precision.
        // The solutions returned by SA have expected_cost based on ~100 samples.
        // We need 100,000 samples to reliably compare with OBD_S and report gaps.
        #pragma omp parallel for
        for(size_t i=0; i<robust_pool.size(); ++i) {
             robust_pool[i].expected_cost = MonteCarlo::expectedCost(
                 instance, robust_pool[i], result.OBD, 
                 100000, // Force high precision for final report
                 sa_params.mc_k, sa_params.seed
             );
        }

        // Sort pool by the new, accurate Expected Cost to find the single best stochastic solution (OBS)
        sort(robust_pool.begin(), robust_pool.end(), [](const Solution &a, const Solution &b)
             { return a.expected_cost < b.expected_cost; });

        if(!robust_pool.empty()) {
            Solution &best_sol = robust_pool.front();
            result.OBS = best_sol;
            
            // Calculate Average Cost of the robust pool (OAS)
            double sum = 0.0;
            for (const auto &sol : robust_pool) {
                sum += sol.expected_cost;
            }
            result.OAS = sum / robust_pool.size();
            
            Writer::writeSolution(baseDir + "/solution_stage2_stoch_sa.txt", result.OBS, sa_params.seed);
        }
    }

    // Save Parameters and Generate Charts (Python Integration)
    Writer::saveParameters(ga_params, sa_params, baseDir + "/parameters.txt");
    Writer::createChart(instance_name);

    return result;
}

void Solver::solveAllInDirectory(const string &dir_path, const GAParams &ga_params,
                                 const SAParams &sa_params)
{
    vector<string> files;

    // 1. Gather Files
    if (ga_params.test_mode)
    {
        files = test_files;
        cout << "Test mode enabled. Using predefined test files." << endl;
    }
    else
    {
        try
        {
            if (!fs::exists(dir_path)) {
                cerr << "Dir not found: " << dir_path << endl;
                return;
            }
            for (const auto &entry : fs::directory_iterator(dir_path)) {
                if (entry.path().extension() == ".txt" || entry.path().extension() == ".dat")
                    files.push_back(entry.path().string());
            }
        }
        catch (...) { return; }
        
        // Sort files naturally (alphanumeric sort would be better, but standard sort is fine)
        sort(files.begin(), files.end());
    }
    
    int total = files.size();
    cout << "Found " << total << " instances. Starting Pipeline..." << endl;

    // 2. Prepare Global Summary CSV
    Writer::ensureDirectory("results");
    Writer::cleanUpDirectory("results"); // Careful: this wipes previous results in 'results/' root
    
    // Write header only once
    Writer::appendCSV("results/summary.csv", 
        "Instance,Best,OBD,OBD_Gap(%),OBD_S,OBD_S_Gap(%),OBS,OBS_Gap(%),OBD_t(ms),OBS_t(ms)", "");

    // 3. Batch Loop
    for (int i = 0; i < total; ++i)
    {
        string file = files[i];
        string instance_name = fs::path(file).stem().string();

        // Simple Console Progress Bar
        float progress = (float)(i + 1) / total;
        int barWidth = 40;
        cout << "\r[";
        int pos = barWidth * progress;
        for (int b = 0; b < barWidth; ++b) {
            if (b < pos) cout << "=";
            else if (b == pos) cout << ">";
            else cout << " ";
        }
        cout << "] " << int(progress * 100.0) << "% " << instance_name << "   ";
        cout.flush();

        // Solve Instance
        PipelineResult result = solveInstance(file, ga_params, sa_params);

        // Calculate Gaps vs Literature (if available)
        stringstream row;
        long literature_result = 0;
        if(BEST.count(instance_name)) 
            literature_result = BEST.at(instance_name);

        double OBD_gap = 0.0, OBD_S_gap = 0.0, OBS_gap = 0.0;
        
        if(literature_result > 0) {
            OBD_gap = (result.OBD.total_cost - literature_result) / (double)literature_result * 100.0;
            OBD_S_gap = (result.OBD_S - literature_result) / (double)literature_result * 100.0;
            OBS_gap = (result.OBS.expected_cost - literature_result) / (double)literature_result * 100.0;
        }

        row << instance_name << ","
            << literature_result << ","
            << result.OBD.total_cost << ","
            << fixed << setprecision(2) << OBD_gap << ","
            << fixed << setprecision(2) << result.OBD_S << ","
            << fixed << setprecision(2) << OBD_S_gap << ","
            << fixed << setprecision(2) << result.OBS.expected_cost << ","
            << fixed << setprecision(2) << OBS_gap << ","
            << fixed << setprecision(2) << result.OBD_t << ","
            << fixed << setprecision(2) << result.OBS_t;
            
        Writer::appendCSV("results/summary.csv", "", row.str());
    }
    cout << endl << "Batch run complete. Check 'results/summary.csv'." << endl;
}

pair<vector<Solution>, GaRunMetrics> Solver::solveDeterministic(const Instance &instance, const GAParams &ga_params)
{
    // Executes the GA (Robust Version)
    GA ga(ga_params, instance);
    auto ga_res = ga.run(instance);
    vector<Solution> &solutions = ga_res.first;
    GaRunMetrics &metrics = ga_res.second;

    // Prepare Directory for instance
    fs::path p(metrics.instance_name);
    string baseDir = "results/" + p.stem().string();
    Writer::ensureDirectory(baseDir);

    // Write Detailed History (Critical for Charts)
    // We overwrite history_ga.csv to ensure clean data for charts
    Writer::cleanUpFile(baseDir + "/history_ga.csv");

    // We iterate through history to save diversity metrics (StdDev, UniqueRatio)
    for (const auto &gen : metrics.history)
    {
        stringstream row;
        row << gen.generation_index << "," 
            << gen.best_cost << "," 
            << gen.avg_cost << ","
            << gen.time_total_ms << "," 
            << gen.ls_improvements << "," 
            << gen.avg_open_facilities << "," 
            << gen.duplicates << "," 
            // Save Robust Metrics for Python analysis
            << gen.cost_std_dev << "," 
            << gen.unique_ratio;
            
        Writer::appendCSV(baseDir + "/history_ga.csv", 
            "Gen,Best,Avg,Time,Improv,Open,Duplicates,StdDev,UniqueRatio", row.str());
    }

    return pair(solutions, metrics);
}

vector<Solution> Solver::solveStochastic(const Instance &instance, const SAParams &sa_params,
                                         const vector<Solution> &initialPool, const Solution &best_deter_sol)
{
    SA sa(sa_params);
    vector<Solution> robust_pool;

    CostEstimator stoch_estimator = [&](const Instance &inst, const Solution &sol) -> double
    {
        return MonteCarlo::expectedCost(inst, sol, best_deter_sol, 
                                        sa_params.mc_samples, sa_params.mc_k, sa_params.seed);
    };
    
    // Obter nome da instância para o Writer
    fs::path p(instance.filePath);
    string instance_name = p.stem().string();

    int run_id = 0;
    for (const auto &sol : initialPool)
    {
        // Chama refine e pega o par {Solução, Histórico}
        auto result_pair = sa.refine(instance, sol, stoch_estimator);
        
        robust_pool.push_back(result_pair.first);
        
        // Salva o histórico desta execução específica
        Writer::saveSaStats(instance_name, run_id, result_pair.second);
        
        run_id++;
    }
    return robust_pool;
}