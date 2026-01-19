#pragma once

#include <string>
#include "model/Solution.h"
#include <map>
#include "util/Metrics.h"

using namespace std;

class Writer {
public:
    // Ensure a directory exists (creates it if missing)
    static void ensureDirectory(const string& path);

    // Write a human-readable solution summary to a text file.
    static bool writeSolution(const string& path, const Solution& sol, unsigned long long seed);

    // Convert solution to a string representation.
    static string solToString(const Solution& sol, unsigned long long seed);

    // Append a CSV row. Creates file with header if missing.
    static bool appendCSV(const string& csv_path, const string& header_if_new, const string& row);

    // Clean up old results in the specified base directory
    static void cleanUpDirectory(const string& baseDir);

    static void cleanUpFile(const string& filePath);

    static void saveParameters(const GAParams& ga_params, const SAParams& sa_params, const string& path);

    static void createChart(const string& instance_name="");
};