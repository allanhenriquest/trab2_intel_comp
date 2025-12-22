#pragma once

#include <string>
#include "core/Instance.h"

using namespace std;

// Parser for MED-style instance files found in instancias_MED/.
// Responsibility: read file and construct an internal Instance.
// Note: If opening costs are not explicit, this may synthesize them
// based on a heuristic (to be defined/confirmed).
class InstanceMEDParser {
public:
    // Parse a MED-formatted file and return an Instance.
    // Throws std::runtime_error on parsing errors.
    Instance parseFile(const string& filepath) const;

private:
    // Internal helpers to infer opening costs if needed.
    vector<double> synthesizeOpeningCosts(const Instance& base) const;
};
