#include "Writer.h"
#include <fstream>
#include <algorithm>
#include <sstream>

using namespace std;

string Writer::solToString(const Solution& sol){
    stringstream out;
    out << "total_cost: " << sol.total_cost << "\n";
    out << "open_count: " << count(sol.openFacilities.begin(), sol.openFacilities.end(), true) << "\n";
    out << "open_facilities: ";
    for(int i=0; i<(int)sol.openFacilities.size(); ++i)
        out << (sol.openFacilities[i] ? to_string(i+1) : "") << " ";
    out << "\nseed: " << sol.seed << "\n";
    return out.str();
}

bool Writer::writeSolution(const string& path, const Solution& sol){
    ofstream out(path);
    if (!out) return false;
    out << solToString(sol);
    return true;
}

bool Writer::appendCSV(const string& csv_path, const string& header_if_new, const string& row){
    ifstream check(csv_path);
    bool exists = (bool)check;
    check.close();
    ofstream out(csv_path, ios::app);
    if (!out) return false;
    if (!exists) out << header_if_new << "\n";
    out << row << "\n";
    return true;
}
