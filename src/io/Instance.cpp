#include "Instance.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <limits>

using namespace std;

Instance::Instance(const string& path, bool verbose) : filePath(path) {
    ifstream in(path);
    if (!in) {
        if (verbose) cerr << "Error: Cannot open " << path << endl;
        n = 0; m = 0;
        return;
    }

    string token;
    // Peek at the first word
    in >> token;
    
    // If it's "FILE:", consume the next token (filename) and move on
    if (token == "FILE:") {
        string filename_dummy;
        in >> filename_dummy;
        // Now read N
        in >> n;
    } else {
        // It was a number (N), so parse it
        try {
            n = stoi(token);
        } catch (...) {
            if (verbose) cerr << "Error: Expected N at start of file." << endl;
            n = 0; return;
        }
    }

    // Read M and Dummy
    int dummy;
    in >> m >> dummy;

    if (n <= 0 || m <= 0) {
        if (verbose) cerr << "Error: Invalid dimensions (n=" << n << ", m=" << m << ")" << endl;
        n = 0; m = 0;
        return;
    }

    // Resize vectors
    opening_costs.resize(n);
    allocation_costs.resize(n, vector<double>(m));

    // Parse loop
    // Format: [FacilityIndex] [OpeningCost] [CostToClient1] ...
    for (int i = 0; i < n; ++i) {
        int index;
        if (!(in >> index >> opening_costs[i])) {
            if (verbose) cerr << "Error reading facility " << i << endl;
            break;
        }
        
        for (int j = 0; j < m; ++j) {
            in >> allocation_costs[i][j];
        }
    }

    if (verbose) cout << "Loaded Instance: " << n << " facilities, " << m << " customers." << endl;
}