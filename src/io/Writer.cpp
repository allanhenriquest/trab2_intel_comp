#include "Writer.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <filesystem> // Requires C++17
#include <iostream>

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

bool Writer::writeSolution(const string& path, const Solution& sol, unsigned long long seed) {
    ofstream out(path);
    if (!out) return false;
    out << solToString(sol, seed);
    return true;
}

string Writer::solToString(const Solution& sol, unsigned long long seed) {
    stringstream ss;
    ss << "Total Cost: " << fixed << setprecision(2) << sol.total_cost << "\n";
    ss << "Open Facilities (" << sol.openFacilities.size() << " total):\n";
    for (size_t i = 0; i < sol.openFacilities.size(); ++i) {
        if (sol.openFacilities[i]) 
            ss << i << " ";
    }
    ss << "\n";
    ss << "Random Seed: " << seed << "\n";
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