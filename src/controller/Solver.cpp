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

const vector<string> test_files = {
    "./instancias_MED/500-10.txt", "./instancias_MED/500-100.txt", "./instancias_MED/500-1000.txt",
    "./instancias_MED/1500-10.txt", "./instancias_MED/1500-100.txt", "./instancias_MED/1500-1000.txt",
    "./instancias_MED/2500-10.txt", "./instancias_MED/2500-100.txt", "./instancias_MED/2500-1000.txt"};

PipelineResult Solver::solveInstance(const string &instance_path, const GAParams &ga_params,
                                     const SAParams &sa_params)
{
    // Load instance (verbose=true to see parsing errors)
    Instance instance(instance_path, true);
    fs::path p(instance.filePath);
    string instance_name = p.stem().string();
    string baseDir = "results/" + instance_name;

    // SAFETY CHECK
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

    // --- STAGE 1: Deterministic GA ---
    auto ga_res = solveDeterministic(instance, ga_params);
    vector<Solution> &ga_pool = ga_res.first;
    GaRunMetrics &ga_metrics = ga_res.second;

    result.OBD_t = ga_metrics.total_time_ms;

    if (ga_pool.empty())
    {
        result.OBD = Solution(instance.n, instance.m);
        result.OBS = Solution(instance.n, instance.m);
        result.OAS = 0.0;
        return result;
    }
    result.OBD = ga_pool.front();
    result.OBD.expected_cost = MonteCarlo::expectedCost(instance, result.OBD, result.OBD, 
                        100000, sa_params.mc_k, sa_params.seed);
    result.OBD_S = result.OBD.expected_cost;

    if (!ga_pool.empty())
    {
        Writer::writeSolution(baseDir + "/solution_stage1_ga.txt", result.OBD, ga_params.seed);
    }

    // cout << "\n----- AG ELITE POOL -----\n";
    // for (int i=0; i< ga_pool.size(); ++i){
    //     cout << "Solution " << i << ": ";
    //     cout << Writer::solToString(ga_pool[i], ga_params.seed, false); 
    //     cout << endl;
    // }
    // cout << endl;

    // --- STAGE 2: Stochastic SA (10 Runs) ---

    if (!sa_params.solve)
    {
        result.OBS = Solution(instance.n, instance.m);
        result.OAS = 0.0;
        result.OBS_t = 0.0;
    }
    else
    {
        Writer::ensureDirectory(baseDir);
        Timer sa_timer;
        sa_timer.start();
        vector<Solution> robust_pool = solveStochastic(instance, sa_params, ga_pool, result.OBD);
        sa_timer.stop();
        result.OBS_t = sa_timer.elapsedMs();

        // Sort to find best in this pool (using expected_cost from SA)
        sort(robust_pool.begin(), robust_pool.end(), [](const Solution &a, const Solution &b)
             { return a.expected_cost < b.expected_cost; });

        Solution &best_sol = robust_pool.front();
        result.OBS = best_sol;
        Writer::writeSolution(baseDir + "/solution_stage2_stoch_sa.txt", result.OBS, sa_params.seed);

        // Calculate Average
        double sum = 0.0;
        for (const auto &sol : robust_pool) {
            sum += sol.expected_cost;
            cout << "Solution Expected Cost: " << sol.expected_cost << endl;
        }
        result.OAS = sum / robust_pool.size();
    }

    Writer::saveParameters(ga_params, sa_params, baseDir + "/parameters.txt");
    Writer::createChart(instance_name);

    return result;
}

void Solver::solveAllInDirectory(const string &dir_path, const GAParams &ga_params,
                                 const SAParams &sa_params)
{
    vector<string> files;

    if (ga_params.test_mode)
    {
        files = test_files;
        cout << "Test mode enabled. Using predefined test files:" << endl;
        for (auto &file : files)
            cout << file << endl;
        cout << endl;
    }
    else
    {
        try
        {
            if (!fs::exists(dir_path))
            {
                cerr << "Dir not found\n";
                return;
            }
            for (const auto &entry : fs::directory_iterator(dir_path))
            {
                if (entry.path().extension() == ".txt")
                    files.push_back(entry.path().string());
            }
        }
        catch (...)
        {
            return;
        }
        sort(files.begin(), files.end());
        rotate(files.begin(), files.end() - 3, files.end());
    }
    int total = files.size();

    cout << "Found " << total << " instances. Starting Pipeline..." << endl;

    Writer::ensureDirectory("results");
    Writer::cleanUpDirectory("results");
    Writer::appendCSV("results/summary.csv", "Instance,Best,OBD,OBD_Gap(%),OBD_S,OBD_S_Gap(%),OBS,OBS_Gap(%),OBD_t(ms),OBS_t(ms)", "");

    for (int i = 0; i < total; ++i)
    {
        string file = files[i];
        string instance_name = fs::path(file).stem().string();
        cout << "instance_name: " << instance_name << endl;

        // Progress Bar
        float progress = (float)(i + 1) / total;
        int barWidth = 40;
        cout << "\r[";
        int pos = barWidth * progress;
        for (int b = 0; b < barWidth; ++b)
        {
            if (b < pos)
                cout << "=";
            else if (b == pos)
                cout << ">";
            else
                cout << " ";
        }
        cout << "] " << int(progress * 100.0) << "% " << instance_name << "   ";
        cout.flush();

        PipelineResult result = solveInstance(file, ga_params, sa_params);

        stringstream row;
        int literature_result = BEST.at(instance_name);

        double OBD_gap = (result.OBD.total_cost - literature_result) / (double)literature_result * 100.0;
        double OBD_S_gap = (result.OBD_S - literature_result) / (double)literature_result * 100.0;
        double OBS_gap = (result.OBS.expected_cost - literature_result) / (double)literature_result * 100.0;

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
    cout << endl
         << "Batch run complete. Check 'results/' folders." << endl;
    Writer::createChart();
}

pair<vector<Solution>, GaRunMetrics> Solver::solveDeterministic(const Instance &instance, const GAParams &ga_params)
{
    GA ga(ga_params, instance);
    auto ga_res = ga.run(instance);
    vector<Solution> &solutions = ga_res.first;
    GaRunMetrics &metrics = ga_res.second;

    fs::path p(metrics.instance_name);
    string baseDir = "results/" + p.stem().string();
    Writer::cleanUpDirectory(baseDir);
    Writer::ensureDirectory("results");
    Writer::ensureDirectory(baseDir);

    for (const auto &gen : metrics.history)
    {
        stringstream row;
        row << gen.generation_index << "," << gen.best_cost << "," << gen.avg_cost << ","
            << gen.time_total_ms << "," << gen.ls_improvements << "," 
            << gen.avg_open_facilities << "," << gen.duplicates;
        Writer::appendCSV(baseDir + "/history_ga.csv", "Gen,Best,Avg,Time,Improv,Open,Duplicates", row.str());
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
        return MonteCarlo::expectedCost(inst, sol, best_deter_sol, sa_params.mc_samples, sa_params.mc_k, sa_params.seed);
    };

    for (const auto &sol : initialPool)
    {
        robust_pool.push_back(sa.refine(instance, sol, stoch_estimator));
    }
    return robust_pool;
}