#pragma once

#include <string>
#include "model/Solution.h"

using namespace std;

class Writer {
public:
    // Ensure a directory exists (creates it if missing)
    static void ensureDirectory(const string& path);

    // Write a human-readable solution summary to a text file.
    static bool writeSolution(const string& path, const Solution& sol);

    // Convert solution to a string representation.
    static string solToString(const Solution& sol);

    // Append a CSV row. Creates file with header if missing.
    static bool appendCSV(const string& csv_path, const string& header_if_new, const string& row);
};