#include "Writer.h"
#include <fstream>
#include <algorithm>
using namespace std;

bool Writer::writeSolution(const string& path, const Solution& sol) const {
    ofstream out(path);
    if (!out) return false;
    out << "total_cost," << sol.total_cost << "\n";
    out << "open_count," << count(sol.y.begin(), sol.y.end(), true) << "\n";
    return true;
}

bool Writer::appendCSV(const string& csv_path, const string& header_if_new, const string& row) const {
    ifstream check(csv_path);
    bool exists = (bool)check;
    check.close();
    ofstream out(csv_path, ios::app);
    if (!out) return false;
    if (!exists) out << header_if_new << "\n";
    out << row << "\n";
    return true;
}
