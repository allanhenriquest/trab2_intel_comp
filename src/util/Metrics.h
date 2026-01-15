#pragma once
#include <vector>
#include <string>

using namespace std;

// Stores stats for a SINGLE generation
struct GenerationMetrics {
    int generation_index;
    double best_cost;
    double avg_cost;
    long long time_total_ms;        // Time for the whole generation
    long long time_evolution_ms;    // Time for selection/crossover/mutation
    long long time_localsearch_ms;  // Time specifically for "The Auditor"
    int ls_improvements;            // How many times did LS improve a solution?
};

// Stores the summary of a full run on ONE instance
struct RunMetrics {
    string instance_name;
    int n_facilities;
    double final_cost;
    long long total_time_ms;
    vector<GenerationMetrics> history; // The generation-by-generation breakdown
};