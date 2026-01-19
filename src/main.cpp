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
    random_device rd2;
    sa_params.seed = rd2();

    try {
        if (mode == "-i") {
            cout << "--- Single Instance Mode ---" << endl;
            cout << "Target: " << path << endl;
            
            // Run Pipeline
            PipelineResult res = Solver::solveInstance(path, ga_params, sa_params);
            string instance_name = filesystem::path(path).stem().string();
            long literature_result = BEST.at(instance_name);
            double OBD_Gap = (res.OBD.total_cost - literature_result) / (double)literature_result * 100.0;
            double OBD_S_Gap = (res.OBD_S - literature_result) / (double)literature_result * 100.0;
            double OBS_Gap = (res.OBS.expected_cost - literature_result) / (double)literature_result * 100.0;


            cout << "\n--- Final Results ------------------------" << endl;
            cout << left << setw(8) << "Stage" << "Cost" << endl;
            cout << "------------------------------------------" << endl;
            cout << left << setw(8) << "BKS" << literature_result << endl;
            cout << left << setw(8) << "OBD"  << res.OBD.total_cost << endl;
            cout << left << setw(8) << "- gap(%)" << OBD_Gap << endl   ;
            cout << left << setw(8) << "OBD_S" << res.OBD_S << endl;
            cout << left << setw(8) << "- gap(%)" << OBD_S_Gap << endl   ;
            cout << left << setw(8) << "OBS" << res.OBS.expected_cost << endl;
            cout << left << setw(8) << "- gap(%)" << OBS_Gap << endl   ;
            cout << left << setw(8) << "OAS" << res.OAS << endl;
            cout << "--- Times --------------------------------" << endl;
            cout << left << setw(8) << "Stage" << "Time (ms)" << endl;
            cout << "------------------------------------------" << endl;
            cout << left << setw(8) << "OBD_t" << res.OBD_t << endl;
            cout << left << setw(8) << "OBS_t" << res.OBS_t << endl;
            cout << "------------------------------------------" << endl;
            cout << left << setw(8) << "Seed" << ga_params.seed << endl;
            cout << "Check results/ folder for logs." << endl;
        } 
        else if (mode == "-all") {
            cout << "--- Batch Directory Mode ---" << endl;
            Solver::solveAllInDirectory(path, ga_params, sa_params);
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