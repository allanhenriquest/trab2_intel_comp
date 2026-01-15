#include <iostream>
#include <string>
#include <iomanip>
#include "controller/Solver.h"

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
    SAParams sa_params; 
    int mc_samples = 100;
    int mc_k = 5;

    try {
        if (mode == "-i") {
            cout << "--- Single Instance Mode ---" << endl;
            cout << "Target: " << path << endl;
            
            // Run Pipeline
            PipelineResult res = Solver::solveInstance(path, ga_params, sa_params, mc_samples, mc_k);
            
            cout << "\n--- Final Results ------------------------" << endl;
            cout << left << setw(28) << "Stage" << "Cost" << endl;
            cout << "------------------------------------------" << endl;
            cout << left << setw(28) << "1. Genetic Algo (GA)" << fixed << setprecision(2) << res.stage1_ga.total_cost << endl;
            cout << left << setw(28) << "2. Deterministic SA" << res.stage2_det_sa.total_cost << endl;
            cout << left << setw(28) << "3. Stochastic SA (Best)" << res.stage3_stoch_sa.total_cost << endl;
            cout << left << setw(28) << "3. Stochastic SA (Avg)" << res.stage3_avg_cost << endl;
            cout << "------------------------------------------" << endl;
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