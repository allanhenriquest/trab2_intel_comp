#include "Writer.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <filesystem> // Requires C++17
#include <iostream>

#if defined(_WIN32) || defined(_WIN64)
    #define OS_WINDOWS
#elif defined(__linux__) || (defined(__APPLE__) && defined(__MACH__))
    #define OS_UNIX
#else
    #define OS_UNKNOWN
#endif

namespace fs = std::filesystem;
using namespace std;

void Writer::ensureDirectory(const string& path) {
    try {
        if (!fs::exists(path)) {
            fs::create_directories(path);
        }
    } catch (const exception& e) {
        cerr << "Error creating directory " << path << ": " << e.what() << endl;
    }
}

bool Writer::writeSolution(const string& path, const Solution& sol, unsigned long long seed, bool stochastic) {
    ofstream out(path);
    if (!out) return false;
    out << solToString(sol, seed, stochastic);
    return true;
}

string Writer::solToString(const Solution& sol, unsigned long long seed, bool stochastic) {
    stringstream ss;
    ss << "Total cost: " << fixed << setprecision(2) << 
    (stochastic ? sol.expected_cost : sol.total_cost) << "\n";
    ss << "Open facilities: "<< "\n";
    for (size_t i = 0; i < sol.openFacilities.size(); ++i) {
        if (sol.openFacilities[i])
            ss << i << " ";
    }
    ss << "\n";
    ss << "Number of open facilities: " << sol.num_open_facilities << "\n";
    ss << "Random seed: " << seed << "\n";
    return ss.str();
}

bool Writer::appendCSV(const string& csv_path, const string& header_if_new, const string& row) {
    bool file_exists = fs::exists(csv_path);

    ofstream out(csv_path, ios::app);
    if (!out) return false;

    if (!file_exists && !header_if_new.empty()) {
        out << header_if_new << "\n";
    }
    
    if(row.empty()) return true; // Nothing to append
    
    out << row << "\n";
    return true;
}

void Writer::cleanUpDirectory(const string& baseDir) {
    try {
        if (fs::exists(baseDir)) {
            for (const auto& entry : fs::directory_iterator(baseDir)) {
                fs::remove_all(entry.path());
            }
        }
    } catch (const exception& e) {
        cerr << "Error cleaning up directory " << baseDir << ": " << e.what() << endl;
    }
}

void Writer::cleanUpFile(const string& filePath) {
    try {
        if (fs::exists(filePath)) {
            fs::remove(filePath);
        }
    } catch (const exception& e) {
        cerr << "Error deleting file " << filePath << ": " << e.what() << endl;
    }
}

void Writer::saveParameters(const GAParams& ga_params, const SAParams& sa_params, const string& path) {
    stringstream ss;
    ss << "GA Parameters:\n";
    ss << "--------------------\n";
    ss << left << setw(20) << "Test Mode: " << (ga_params.test_mode ? "True" : "False") << "\n";
    ss << left << setw(20) << "Population Size: " << ga_params.pop_size << "\n";
    ss << left << setw(20) << "Max Generations: " << ga_params.max_generations << "\n";
    ss << left << setw(20) << "Elite Ratio: " << ga_params.elite_ratio << "\n";
    ss << left << setw(20) << "Elite Count: " << ga_params.elite_count << "\n";
    ss << left << setw(20) << "Mutation Rate: " << ga_params.mutation_rate << "\n";
    ss << left << setw(20) << "Open Threshold: " << ga_params.open_threshold << "\n";
    ss << left << setw(20) << "Stop Threshold: " << ga_params.stop_threshold << "\n";
    ss << left << setw(20) << "Use Local Search: " << (ga_params.use_local_search ? "True" : "False") << "\n";
    ss << left << setw(20) << "Use Smart Leader: " << (ga_params.use_smart_leader ? "True" : "False") << "\n";
    ss << left << setw(20) <<"Random Seed: " << ga_params.seed << "\n";
    ss << "--------------------\n\n";

    ss << "SA Parameters:\n";
    ss << "--------------------\n";
    ss << left << setw(20) << "Solve: " << (sa_params.solve ? "True" : "False") << "\n";
    ss << left << setw(20) << "Initial Temp (T0): " << sa_params.T0 << "\n";
    ss << left << setw(20) << "Min Temp (Tmin): " << sa_params.Tmin << "\n";
    ss << left << setw(20) << "Alpha: " << sa_params.alpha << "\n";
    ss << left << setw(20) << "Iters per T: " << sa_params.iters_per_T << "\n";
    ss << left << setw(20) << "MC Samples: " << sa_params.mc_samples << "\n";
    ss << left << setw(20) << "MC k: " << sa_params.mc_k << "\n";
    ss << left << setw(20) << "Random Seed: " << sa_params.seed << "\n";
    ss << "--------------------\n";
    
    ofstream out(path);
    if (!out) {
        cerr << "Error writing parameters to " << path << endl;
        return;
    }
    out << ss.str();
    out.close();
}

void Writer::createChart(const string& instance_name) {
    cout << "Creating chart..." << endl;
    if(!fs::exists("./venv"))
    {
        if(system("python3 -m venv venv") != 0)
        {
            cerr << "Error creating virtual environment for chart generation." << endl;
            return;
        }
    }
    if(system("./venv/bin/pip install -r pyRequirements.txt -qq"))
    {
        cerr << "Error installing Python dependencies for chart generation." << endl;
        return;
    }
    if(system(("./venv/bin/python src/analysis/generateCharts.py " + instance_name).c_str()))
    {
        cerr << "Error generating charts using Python script." << endl;
        return;
    }
    cout << "Chart created successfully." << endl;
}