#include <iostream>
#include <string>
#include <iomanip>
#include "controller/Solver.h"
#include <filesystem>
#include <random>

using namespace std;

void printUsage() {
    cerr << "Usage:" << endl;
    cerr << "  Single Instance: uflp -i <instance_path>" << endl;
    cerr << "  Batch Directory: uflp -all <directory_path>" << endl;
}

int main(int argc, char** argv) {
    if (argc < 3) {
        printUsage();
        return 1;
    }

    string mode = argv[1];
    string path = argv[2];

    GAParams ga_params; 
    random_device rd;
    ga_params.seed = rd();

    SAParams sa_params; 
    int mc_samples = 100;
    int mc_k = 5;

    try {
        if (mode == "-i") {
            cout << "--- Single Instance Mode ---" << endl;
            cout << "Target: " << path << endl;
            
            // Run Pipeline
            PipelineResult res = Solver::solveInstance(path, ga_params, sa_params, mc_samples, mc_k);
            string instance_name = filesystem::path(path).stem().string();
            double literature_result = BEST.at(instance_name);
            double GA_gap = (res.best_ga.total_cost - literature_result) / literature_result * 100.0;

            cout << "\n--- Final Results ------------------------" << endl;
            cout << left << setw(28) << "Stage" << "Cost" << endl;
            cout << "------------------------------------------" << endl;
            cout << left << setw(28) << "1. Genetic Algorithm (GA)"  << res.best_ga.total_cost << endl;
            cout << left << setw(28) << "   - Literature Solution" << literature_result << endl;
            cout << left << setw(28) << "   - gap(%)" << GA_gap << endl   ;
            cout << left << setw(28) << "2. Stochastic SA (Best)" << res.best_stoch_sa.total_cost << endl;
            cout << left << setw(28) << "2. Stochastic SA (Avg)" << res.stoch_sa_avg_cost << endl;
            cout << "--- Times --------------------------------" << endl;
            cout << left << setw(28) << "Stage" << "Time (ms)" << endl;
            cout << "------------------------------------------" << endl;
            cout << left << setw(28) << "1. Genetic Algo (GA)" << res.ga_time_ms << endl;
            cout << left << setw(28) << "2. Stochastic SA" << res.stoch_sa_time_ms << endl;
            cout << "------------------------------------------" << endl;
            cout << left << setw(28) << "Seed" << ga_params.seed << endl;
            cout << "Check results/ folder for logs." << endl;
        } 
        else if (mode == "-all") {
            cout << "--- Batch Directory Mode ---" << endl;
            Solver::solveAllInDirectory(path, ga_params, sa_params, mc_samples, mc_k);
        } 
        else {
            printUsage();
            return 1;
        }
    } catch (const exception& ex) {
        cerr << "Error: " << ex.what() << endl;
        return 2;
    }

    return 0;
}