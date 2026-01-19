#include "Writer.h"
#include <filesystem>
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

void Writer::saveGaStats(const Instance& inst, const GaRunMetrics& metrics, std::string subfolder) {
    // 1. Base Path: results/<instance_name>/
    std::string instName = fs::path(inst.filePath).stem().string();
    fs::path baseDir = fs::path("results") / instName;
    
    // 2. Append subfolder if provided
    if (!subfolder.empty()) {
        baseDir /= subfolder;
    }

    // 3. Create Directories
    if (!fs::exists(baseDir)) {
        fs::create_directories(baseDir);
    }

    // 4. Write CSV
    fs::path csvPath = baseDir / "ga_metrics.csv";
    bool fileExists = fs::exists(csvPath);
    
    // Abrir arquivo UMA VEZ
    std::ofstream out(csvPath, std::ios::app); 
    
    if (!out.is_open()) {
        std::cerr << "[Writer] Error opening file: " << csvPath << std::endl;
        return;
    }

    // Cabeçalho com as NOVAS colunas
    if (!fileExists) {
        out << "Generation,BestCost,AvgCost,AvgOpen,StdDevCost,UniqueRatio\n";
    }
    
    // Escrever dados com as NOVAS métricas
    for (const auto& gm : metrics.history) {
        out << gm.generation_index << "," 
            << gm.best_cost << "," 
            << gm.avg_cost << "," 
            << gm.avg_open_facilities << ","
            << gm.cost_std_dev << ","      // Novo
            << gm.unique_ratio << "\n";    // Novo
    }
    
    std::cout << "[Writer] GA metrics saved to " << csvPath << std::endl;
}