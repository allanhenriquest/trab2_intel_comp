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
        fail.best_ga = Solution(0, 0);
        fail.best_stoch_sa = Solution(0, 0);
        fail.stoch_sa_avg_cost = 0.0;
        return fail;
    }

    PipelineResult result;

    // --- STAGE 1: Deterministic GA ---
    auto ga_res = solveDeterministic(instance, ga_params);
    vector<Solution> &ga_pool = ga_res.first;
    GaRunMetrics &ga_metrics = ga_res.second;

    result.ga_time_ms = ga_metrics.total_time_ms;

    if (ga_pool.empty())
    {
        result.best_ga = Solution(instance.n, instance.m);
        result.best_stoch_sa = Solution(instance.n, instance.m);
        result.stoch_sa_avg_cost = 0.0;
        return result;
    }
    result.best_ga = ga_pool.front();

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
        result.best_stoch_sa = result.best_ga;
        result.stoch_sa_avg_cost = result.best_ga.total_cost;
        result.stoch_sa_time_ms = 0.0;
    }
    else
    {
        Writer::ensureDirectory(baseDir);
        Timer sa_timer;
        sa_timer.start();
        vector<Solution> robust_pool = solveStochastic(instance, sa_params, ga_pool, result.best_ga);
        sa_timer.stop();
        result.stoch_sa_time_ms = sa_timer.elapsedMs();

        // Sort to find best in this pool (using expected_cost from SA)
        sort(robust_pool.begin(), robust_pool.end(), [](const Solution &a, const Solution &b)
             { return a.expected_cost < b.expected_cost; });

        Solution &best_sol = robust_pool.front();
        result.best_stoch_sa = best_sol;
        Writer::writeSolution(baseDir + "/solution_stage2_stoch_sa.txt", result.best_stoch_sa, sa_params.seed, true);

        // Calculate Average
        double sum = 0.0;
        for (const auto &sol : robust_pool) {
            sum += sol.expected_cost;
            cout << "Solution Expected Cost: " << sol.expected_cost << endl;
        }
        result.stoch_sa_avg_cost = sum / robust_pool.size();
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
    Writer::appendCSV("results/summary.csv", "Instance,Best,GA,GA_Gap(%),Stoch_SA,Stoch_SA_Gap(%),GA_Time(ms),Stoch_SA_Time(ms)", "");

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

        double ga_gap = (result.best_ga.total_cost - literature_result) / (double)literature_result * 100.0;
        double sa_gap = (result.best_stoch_sa.expected_cost - literature_result) / (double)literature_result * 100.0;

        row << instance_name << ","
            << literature_result << ","
            << result.best_ga.total_cost << ","
            << fixed << setprecision(2) << ga_gap << ","
            << fixed << setprecision(2) << result.best_stoch_sa.expected_cost << ","
            << fixed << setprecision(2) << sa_gap << ","
            << fixed << setprecision(2) << result.ga_time_ms << ","
            << fixed << setprecision(2) << result.stoch_sa_time_ms;
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

    if (!solutions.empty())
    {
        Writer::writeSolution(baseDir + "/solution_stage1_ga.txt", solutions.front(), ga_params.seed, false);
    }

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