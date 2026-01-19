#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include "io/Instance.h"
#include "controller/Solver.h"

namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
    std::string instance_path;
    bool run_all = false;
    std::string instances_dir;
    
    // Default configurations
    bool ga_only = false;
    bool use_ls = true;
    bool use_sl = true;

    // Parse Args
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-i" && i + 1 < argc) {
            instance_path = argv[++i];
        } else if (arg == "-all" && i + 1 < argc) {
            run_all = true;
            instances_dir = argv[++i];
        } 
        // New flags
        else if (arg == "--ga-only") {
            ga_only = true;
        } else if (arg == "--ls" && i + 1 < argc) {
            use_ls = (std::stoi(argv[++i]) == 1);
        } else if (arg == "--sl" && i + 1 < argc) {
            use_sl = (std::stoi(argv[++i]) == 1);
        }
    }

    Solver solver;
    // Configure solver with user choices
    solver.configure(ga_only, use_ls, use_sl);

    if (run_all) {
        if (!fs::exists(instances_dir)) {
            std::cerr << "Error: Directory not found: " << instances_dir << std::endl;
            return 1;
        }
        for (const auto& entry : fs::directory_iterator(instances_dir)) {
             if (entry.path().extension() == ".txt" || entry.path().extension() == ".dat") {
                 std::cout << "\n>>> Processing: " << entry.path().filename() << std::endl;
                 try {
                     Instance inst(entry.path().string());
                     solver.solve(inst);
                 } catch (const std::exception& e) {
                     std::cerr << "Error: " << e.what() << std::endl;
                 }
             }
        }
    } else if (!instance_path.empty()) {
        std::cout << ">>> Processing Single Instance: " << instance_path << std::endl;
        Instance inst(instance_path);
        solver.solve(inst);
    } else {
        std::cerr << "Usage: ./uflp -i <file> | -all <dir> [--ga-only] [--ls 0/1] [--sl 0/1]" << std::endl;
        return 1;
    }

    return 0;
}