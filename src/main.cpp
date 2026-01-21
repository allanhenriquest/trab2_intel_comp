#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <iomanip> // Necessário para setprecision
#include "io/Instance.h"
#include "controller/Solver.h"
#include "util/Metrics.h"

namespace fs = std::filesystem;

void printUsage() {
    std::cerr << "Usage: ./uflp [OPTIONS]\n"
              << "Options:\n"
              << "  -i <file>        Run single instance\n"
              << "  -all <dir>       Run all instances in directory\n"
              << "  --stats          Run Statistical Analysis (30 runs per instance)\n"
              << "  --ga-only        Disable SA stage (Deterministic only)\n"
              << "  --ls <0/1>       Enable/Disable Local Search (Default: 1)\n"
              << "  --sl <0/1>       Enable/Disable Smart Leader (Default: 1)\n"
              << "  --seed <int>     Set fixed seed (Default: 42)\n"
              << "  --samples <int>  Monte Carlo samples for SA search (Default: 100)\n"
              << "  --gen <int>      Max GA Generations (Default: 300)\n"
              << "  --k <int>        Uncertainty Factor k (5=Low, 10=Med, 20=High) [Default: 5]\n";
}

int main(int argc, char* argv[]) {
    std::string instance_path;
    std::string instances_dir;
    bool run_all = false;
    bool run_stats = false;
    int stats_runs = 30;
    
    GAParams ga_params;
    SAParams sa_params;
    
    ga_params.seed = 42;
    sa_params.seed = 42;
    sa_params.mc_samples = 100; 
    sa_params.mc_k = 5;         

    // Parse Args
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-i" && i + 1 < argc) {
            instance_path = argv[++i];
        } 
        else if (arg == "-all" && i + 1 < argc) {
            run_all = true;
            instances_dir = argv[++i];
        } 
        else if (arg == "--stats") {
            run_stats = true;
            run_all = true; // Stats implica rodar num diretório
        }
        else if (arg == "--ga-only") {
            sa_params.solve = false;
        } 
        else if (arg == "--ls" && i + 1 < argc) {
            ga_params.use_local_search = (std::stoi(argv[++i]) == 1);
        } 
        else if (arg == "--sl" && i + 1 < argc) {
            ga_params.use_smart_leader = (std::stoi(argv[++i]) == 1);
        }
        else if (arg == "--seed" && i + 1 < argc) {
            unsigned long long s = std::stoull(argv[++i]);
            ga_params.seed = s;
            sa_params.seed = s;
        }
        else if (arg == "--samples" && i + 1 < argc) {
            sa_params.mc_samples = std::stoi(argv[++i]);
        }
        else if (arg == "--gen" && i + 1 < argc) {
            ga_params.max_generations = std::stoi(argv[++i]);
        }
        else if (arg == "--k" && i + 1 < argc) {
            sa_params.mc_k = std::stoi(argv[++i]); 
        }
        else if (arg == "--help" || arg == "-h") {
            printUsage();
            return 0;
        }
    }

    Solver solver;

    // LÓGICA DE EXECUÇÃO
    if (run_stats) {
        // MODO ESTATÍSTICO (30 Runs)
        if (instances_dir.empty()) {
            // Se o usuário não passou -all <dir> antes de --stats, tenta assumir default ou erro
             if (!fs::exists(instances_dir)) {
                std::cerr << "Error: Directory needed for stats mode. Use: -all <dir> --stats" << std::endl;
                return 1;
            }
        }
        
        if (!fs::exists(instances_dir)) {
            std::cerr << "Error: Directory not found: " << instances_dir << std::endl;
            return 1;
        }
        
        solver.runStatisticalAnalysis(instances_dir, ga_params, sa_params, stats_runs);
    } 
    else if (run_all) {
        if (!fs::exists(instances_dir)) {
            std::cerr << "Error: Directory not found: " << instances_dir << std::endl;
            return 1;
        }
        solver.solveAllInDirectory(instances_dir, ga_params, sa_params);
    } 
    else if (!instance_path.empty()) {
        // MODO SINGLE INSTANCE
        std::cout << ">>> Processing Single Instance: " << instance_path << " (k=" << sa_params.mc_k << ")" << std::endl;
        PipelineResult res = solver.solveInstance(instance_path, ga_params, sa_params);
        
        std::cout << "Final Results:\n";
        
        std::cout << std::fixed << std::setprecision(0); 
        
        std::cout << "  OBD (Deterministic): " << (double)res.OBD.total_cost << "\n";
        std::cout << "  OBD Expected (k=" << sa_params.mc_k << "): " << res.OBD_S << "\n";
        if(sa_params.solve)
            std::cout << "  OBS (Stochastic, k=" << sa_params.mc_k << "): " << res.OBS.expected_cost << "\n";
            
        std::cout.unsetf(std::ios_base::fixed);
            
    } else {
        printUsage();
        return 1;
    }

    return 0;
}