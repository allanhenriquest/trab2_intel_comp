#include "Solver.h"
#include <iostream>
#include <string>
#include <filesystem>
#include <iomanip>

namespace fs = std::filesystem;

Solver::Solver() {}

void Solver::configure(bool ga_only, bool use_ls, bool use_sl) {
    ga_only_ = ga_only;
    use_ls_ = use_ls;
    use_sl_ = use_sl;
}

void Solver::solve(const Instance& instance) {
    // 1. Identify Instance Name for Best Known lookup
    std::string instName = fs::path(instance.filePath).stem().string();
    long optimal = 0;
    if (BEST.count(instName)) {
        optimal = BEST.at(instName);
    }

    // 2. Configure GA Parameters
    GAParams ga_params;
    
    // Flags configuradas via CLI/Menu
    ga_params.use_local_search = use_ls_;
    ga_params.use_smart_leader = use_sl_;
    
    // Configurações manuais do Solver (Sobrescrevem Metrics.h se necessário)
    // ga_params.max_generations = 200; // <--- REMOVIDO: Agora usa o valor de Metrics.h (padrão: 300)
    
    // Mantemos estas sobrescritas pois são específicas da estratégia deste Solver
    ga_params.pop_size = 100;     // Garante tamanho 100
    ga_params.elite_count = 1;    // Mantemos apenas o melhor (Elitismo agressivo)
    ga_params.mutation_rate = 0.01; // Taxa base (será adaptada dinamicamente pelo GA)

    std::cout << "------------------------------------------------\n";
    std::cout << "[Solver] Instance: " << instName << "\n";
    std::cout << "[Solver] Config: GA Only=" << (ga_only_ ? "YES" : "NO")
              << " | LS=" << (use_ls_ ? "ON" : "OFF") 
              << " | SL=" << (use_sl_ ? "ON" : "OFF") << "\n";
    std::cout << "[Solver] Generations: " << ga_params.max_generations << "\n"; // Log para confirmar
    if(optimal > 0) std::cout << "[Solver] Known Optimal: " << optimal << "\n";
    std::cout << "------------------------------------------------\n";

    // 3. Run GA
    GA ga(ga_params, instance);
    auto [population, ga_metrics] = ga.run(instance);
    
    Solution best_ga = population[0];

    // 4. Calculate and Print Gap
    double gap = 0.0;
    if (optimal > 0) {
        gap = 100.0 * (double)(best_ga.total_cost - optimal) / (double)optimal;
        std::cout << ">>> GA Result: " << best_ga.total_cost 
                  << " (Gap: " << std::fixed << std::setprecision(2) << gap << "%)\n";
    } else {
        std::cout << ">>> GA Result: " << best_ga.total_cost << " (Optimal unknown)\n";
    }

    // 5. Handle Output Folders
    std::string output_subfolder;
    
    if (ga_only_) {
        // --- GA ONLY MODE ---
        // Create descriptive folder name: e.g., "GA_TEST_LS-ON_SL-OFF"
        output_subfolder = "GA_TEST";
        output_subfolder += (use_ls_ ? "_LS-ON" : "_LS-OFF");
        output_subfolder += (use_sl_ ? "_SL-ON" : "_SL-OFF");
        
        std::cout << "[Solver] Saving GA stats to: results/" << instName << "/" << output_subfolder << "\n";
        
        // Save only GA stats
        Writer::saveGaStats(instance, ga_metrics, output_subfolder);
        
        return; // Stop here, do not run SA
    } 
    
    // --- FULL PIPELINE MODE (Future Implementation) ---
    output_subfolder = "FULL_PIPELINE";
    
    std::cout << "[Solver] Proceeding to SA (Stage 2)...\n";
    
    // Placeholder for SA logic (requires corrected SA implementation)
    // SAParams sa_params;
    // SA sa(sa_params);
    // ...
    
    // Save GA stats for full pipeline as well
    Writer::saveGaStats(instance, ga_metrics, output_subfolder);
}