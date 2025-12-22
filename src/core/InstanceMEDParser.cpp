#include "InstanceMEDParser.h"
#include <fstream>
#include <sstream>
#include <stdexcept>

using namespace std;

Instance InstanceMEDParser::parseFile(const string& filepath) const {
    // Minimal placeholder: read first line to extract n and m if available.
    ifstream in(filepath);
    if (!in) throw runtime_error("Cannot open instance file: " + filepath);

    // This parser is a stub and should be replaced with real MED parsing.
    // For now, attempt to read two ints from the first non-empty numeric line.
    int n = 0, m = 0;
    {
        string line;
        while (getline(in, line)) {
            istringstream iss(line);
            int a=0, b=0;
            if (iss >> a >> b) {
                // crude heuristic: if there's a third number it's likely a flag, ignore
                n = a; m = b; break;
            }
        }
    }
    if (n <= 0 || m <= 0) throw runtime_error("Failed to read n and m from MED file (stub parser)");
    (void)m; // n == m in MED; we keep single n.

    Instance inst(n);
    // opening costs default to 0 (to be set properly later)
    return inst;
}

vector<double> InstanceMEDParser::synthesizeOpeningCosts(const Instance& base) const {
    // Placeholder: zero opening costs. Replace with domain-specific rule if required.
    return vector<double>(base.n, 0.0);
}

