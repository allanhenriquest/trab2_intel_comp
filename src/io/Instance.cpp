#include "Instance.h"
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <iostream>

using namespace std;

Instance::Instance(const string filePath, bool verbose){
    this->filePath = filePath;

    ifstream file(filePath);
    if (!file) throw runtime_error("Cannot open instance file: " + filePath);

    string line;
    getline(file, line);
    getline(file, line);
    n = stoi(line.substr(line.find_first_of(' ')+1));
    allocation_costs.assign(n, vector<double>(n, 0.0));

    string temp;
    const int barWidth = 50;
    for (int i = 0; i < n; ++i) {
        getline(file, line);
        istringstream iss(line);
        iss >> temp; // skip index
        if(i == 0) iss >> opening_cost; else iss >> temp; // skip opening cost for other lines
        for (int j = 0; j < n; ++j) {
            iss >> allocation_costs[i][j];
        }

        // progress bar
        if(verbose){
            double progress = double(i + 1) / n;
            int pos = int(barWidth * progress);
            cout << "\rReading instance: [";
            for (int k = 0; k < barWidth; ++k) cout << (k < pos ? '#' : ' ');
            cout << "] " << int(progress * 100) << "%";
            cout.flush();
        }
    }
    cout << endl;
}
