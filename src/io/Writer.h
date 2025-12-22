#pragma once

#include <string>
#include "model/Solution.h"

using namespace std;

// Utility to persist results and logs.
class Writer {
public:
    // Write a human-readable solution summary to a text file.
    bool writeSolution(const string& path, const Solution& sol) const;

    // Append a CSV row of metrics. Creates file with header if missing.
    bool appendCSV(const string& csv_path, const string& header_if_new, const string& row) const;
};
