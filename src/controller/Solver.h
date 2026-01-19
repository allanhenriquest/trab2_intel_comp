#pragma once
#include "io/Instance.h"
#include "meta/GA.h"
#include "meta/SA.h"
#include "io/Writer.h"
#include <map>
#include <string>

// Known optimal values for MED instances (for gap calculation)
inline const std::map<std::string, long> BEST = {
    {"500-10", 798577},
    {"500-100", 326790},
    {"500-1000", 99169},
    {"1000-10", 1434154},
    {"1000-100", 607878},
    {"1000-1000", 220560},
    {"1500-10", 2000801},
    {"1500-100", 866454},
    {"1500-1000", 334962},
    {"2000-10", 2558118},
    {"2000-100", 1122748},
    {"2000-1000", 437686},
    {"2500-10", 3101800},
    {"2500-100", 1347516},
    {"2500-1000", 534405},
    {"3000-10", 3570766},
    {"3000-100", 1606969},
    {"3000-1000", 643463}
};

class Solver {
public:
    Solver();
    
    // Configures execution flags (set via CLI/Menu)
    void configure(bool ga_only, bool use_ls, bool use_sl);
    
    // Main execution method
    void solve(const Instance& instance);

private:
    // Configuration State
    bool ga_only_ = false;
    bool use_ls_ = true;
    bool use_sl_ = true;
};