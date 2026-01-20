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

// List of files for Test Mode
const vector<string> test_files = {
    "./instancias_MED/500-10.txt", "./instancias_MED/500-100.txt", "./instancias_MED/500-1000.txt",
    "./instancias_MED/1000-10.txt", "./instancias_MED/1000-100.txt", "./instancias_MED/1000-1000.txt", // <--- Adicionado
    "./instancias_MED/1500-10.txt", "./instancias_MED/1500-100.txt", "./instancias_MED/1500-1000.txt",
    "./instancias_MED/2000-10.txt", "./instancias_MED/2000-100.txt", "./instancias_MED/2000-1000.txt", // <--- Adicionado
    "./instancias_MED/2500-10.txt", "./instancias_MED/2500-100.txt", "./instancias_MED/2500-1000.txt",
    "./instancias_MED/3000-10.txt", "./instancias_MED/3000-100.txt", "./instancias_MED/3000-1000.txt"  // <--- Adicionado
};

PipelineResult Solver::solveInstance(const string &instance_path, const GAParams &ga_params,
                                     const SAParams &sa_params)
{
    // 1. Load Instance
    Instance instance(instance_path, true);
    
    fs::path p(instance.filePath);
    string instance_name = p.stem().string();
    string baseDir = "results/" + instance_name;

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
    // STAGE 1: Deterministic GA
    // ====================================================
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

    // Set OBD
    result.OBD = ga_pool.front();
    
    // Evaluate OBD under Uncertainty (Simulation - High Precision)
    result.OBD.expected_cost = MonteCarlo::expectedCost(instance, result.OBD, result.OBD, 
                        100000, sa_params.mc_k, sa_params.seed);
    result.OBD_S = result.OBD.expected_cost;

    Writer::writeSolution(baseDir + "/solution_stage1_ga.txt", result.OBD, ga_params.seed);

    // ====================================================
    // STAGE 2: Stochastic SA
    // ====================================================

    if (!sa_params.solve)
    {
        result.OBS = result.OBD; 
        result.OAS = result.OBD_S;
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

        // Final Validation (High Precision)
        #pragma omp parallel for
        for(size_t i=0; i<robust_pool.size(); ++i) {
             robust_pool[i].expected_cost = MonteCarlo::expectedCost(
                 instance, robust_pool[i], result.OBD, 
                 100000, 
                 sa_params.mc_k, sa_params.seed
             );
        }

        sort(robust_pool.begin(), robust_pool.end(), [](const Solution &a, const Solution &b)
             { return a.expected_cost < b.expected_cost; });

        if(!robust_pool.empty()) {
            result.OBS = robust_pool.front();
            double sum = 0.0;
            for (const auto &sol : robust_pool) sum += sol.expected_cost;
            result.OAS = sum / robust_pool.size();
            
            Writer::writeSolution(baseDir + "/solution_stage2_stoch_sa.txt", result.OBS, sa_params.seed);
        }
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
        cout << "Test mode enabled." << endl;
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
        sort(files.begin(), files.end());
    }
    
    int total = files.size();
    cout << "Found " << total << " instances. Starting Pipeline..." << endl;

    Writer::ensureDirectory("results");
    Writer::cleanUpDirectory("results"); 
    
    Writer::appendCSV("results/summary.csv", 
        "Instance,Best,OBD,OBD_Gap(%),OBD_S,OBD_S_Gap(%),OBS,OBS_Gap(%),OBD_t(ms),OBS_t(ms)", "");

    for (int i = 0; i < total; ++i)
    {
        string file = files[i];
        string instance_name = fs::path(file).stem().string();

        // Progress Bar
        float progress = (float)(i + 1) / total;
        int barWidth = 30;
        cout << "\r[";
        int pos = barWidth * progress;
        for (int b = 0; b < barWidth; ++b) {
            if (b < pos) cout << "=";
            else if (b == pos) cout << ">";
            else cout << " ";
        }
        
        // --- VISUAL FEEDBACK (TERMINAL) ---
        // Aqui ainda não temos o resultado, então só imprimimos o nome
        cout << "] " << int(progress * 100.0) << "% " << instance_name << " ";
        cout.flush();

        // Executa
        PipelineResult result = solveInstance(file, ga_params, sa_params);

        // --- IMPRESSÃO FORMATADA DOS CUSTOS (Logo após calcular) ---
        // Mostra números "limpos" no terminal para análise rápida
        cout << fixed << setprecision(0); 
        cout << "| OBD: " << result.OBD_S << " | OBS: " << result.OBS.expected_cost;
        cout.unsetf(ios_base::fixed); // Restaura para não afetar outras coisas
        cout.flush();

        // --- SALVAMENTO NO CSV (Alta Precisão) ---
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

        // CSV: Usa defaultfloat (ou precisão alta) para os Custos
        // Usa fixed(2) apenas para os Gaps (porcentagens)
        row << instance_name << ","
            << literature_result << ","
            << result.OBD.total_cost << ","
            << fixed << setprecision(2) << OBD_gap << ","
            << defaultfloat << result.OBD_S << ","         // <--- Custo bruto no CSV
            << fixed << setprecision(2) << OBD_S_gap << ","
            << defaultfloat << result.OBS.expected_cost << "," // <--- Custo bruto no CSV
            << fixed << setprecision(2) << OBS_gap << ","
            << defaultfloat << result.OBD_t << ","
            << result.OBS_t;
            
        Writer::appendCSV("results/summary.csv", "", row.str());
    }
    cout << endl << "Batch run complete." << endl;
}

pair<vector<Solution>, GaRunMetrics> Solver::solveDeterministic(const Instance &instance, const GAParams &ga_params)
{
    GA ga(ga_params, instance);
    auto ga_res = ga.run(instance);
    vector<Solution> &solutions = ga_res.first;
    GaRunMetrics &metrics = ga_res.second;

    fs::path p(metrics.instance_name);
    string baseDir = "results/" + p.stem().string();
    Writer::ensureDirectory(baseDir);
    Writer::cleanUpFile(baseDir + "/history_ga.csv");

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

    CostEstimator stoch_estimator = [&](const Instance &inst, const Solution &sol, int samples) -> double
    {
        return MonteCarlo::expectedCost(inst, sol, best_deter_sol, 
                                        samples, sa_params.mc_k, sa_params.seed);
    };

    fs::path p(instance.filePath);
    string instance_name = p.stem().string();

    int run_id = 0;
    for (const auto &sol : initialPool)
    {
        auto result_pair = sa.refine(instance, sol, stoch_estimator);
        robust_pool.push_back(result_pair.first);
        Writer::saveSaStats(instance_name, run_id, result_pair.second);
        run_id++;
    }
    return robust_pool;
}

void Solver::runStatisticalAnalysis(const string &dir_path, const GAParams &ga_params, const SAParams &sa_params, int runs)
{
    vector<string> files;
    
    // 1. Identificar arquivos no diretório
    try {
        if (!fs::exists(dir_path)) return;
        for (const auto &entry : fs::directory_iterator(dir_path)) {
            if (entry.path().extension() == ".txt")
                files.push_back(entry.path().string());
        }
    } catch (...) { return; }
    
    // Ordenar para garantir consistência na execução
    sort(files.begin(), files.end());

    // 2. Preparar Arquivo de Saída Unificado
    // O arquivo é limpo no início para não misturar experimentos antigos
    string out_file = "results/stats_raw_30runs.csv";
    Writer::ensureDirectory("results");
    
    Writer::cleanUpFile(out_file);
    
    // CABEÇALHO ATUALIZADO COM A COLUNA 'k'
    Writer::appendCSV(out_file, 
        "Instance,RunID,Seed,k,BKS,OBD_Cost,OBD_Time,OBS_Cost,OBS_Time,Gap_OBD(%),Gap_OBS(%)", "");

    cout << ">>> STARTING STATISTICAL ANALYSIS (" << runs << " runs per instance, k=" << sa_params.mc_k << ") <<<" << endl;

    // 3. Loop Principal (Instâncias)
    for (const string &file : files)
    {
        string instance_name = fs::path(file).stem().string();
        
        // Tenta obter o BKS (Best Known Solution) do mapa global/estático BEST
        // Se não existir, assume 0 (o que vai gerar gap 0.0 ou infinito, tratado abaixo)
        long bks = (BEST.count(instance_name)) ? BEST.at(instance_name) : 0;

        cout << "Processing " << instance_name << " ";
        
        // 4. Loop Estatístico (30 Execuções)
        for (int r = 0; r < runs; ++r)
        {
            // Varia a semente para garantir independência estatística
            // A base é a seed passada por parâmetro + deslocamento por iteração
            unsigned long long current_seed = ga_params.seed + r * 12345;

            // Cria cópias locais dos parâmetros para não afetar as próximas iterações
            GAParams local_ga = ga_params;
            local_ga.seed = current_seed;
            
            SAParams local_sa = sa_params;
            local_sa.seed = current_seed;

            // Resolve a instância (GA + SA)
            PipelineResult res = solveInstance(file, local_ga, local_sa);

            // Cálculos de Gap
            double gap_obd = 0.0;
            double gap_obs = 0.0;
            
            if (bks > 0) {
                // Gap Determinístico: (Custo GA - BKS) / BKS
                gap_obd = (res.OBD.total_cost - bks) / (double)bks * 100.0;
                
                // Gap Estocástico: (Custo Esperado SA - BKS) / BKS
                // Lembre-se: Gap positivo é esperado pois incerteza custa caro
                gap_obs = (res.OBS.expected_cost - bks) / (double)bks * 100.0;
            }

            // Monta a linha do CSV
            stringstream row;
            row << instance_name << ","
                << r + 1 << ","               // RunID (1..30)
                << current_seed << ","        // Seed usada
                << local_sa.mc_k << ","       // Nível de incerteza k
                << bks << ","                 // Melhor conhecido
                << res.OBD.total_cost << ","  // Melhor Determinístico (GA)
                << res.OBD_t << ","           // Tempo GA
                << res.OBS.expected_cost << "," // Melhor Estocástico (SA)
                << res.OBS_t << ","           // Tempo SA
                << fixed << setprecision(4) << gap_obd << ","
                << gap_obs;

            Writer::appendCSV(out_file, "", row.str());
            
            // Feedback visual mínimo (um ponto por execução)
            cout << "." << flush;
        }
        cout << " Done." << endl;
    }
    
    cout << "\n>>> Statistical Data Saved to: " << out_file << endl;
}