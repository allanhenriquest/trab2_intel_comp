#pragma once
#include <vector>
#include <string>

using namespace std;

struct GAParams
{
    int pop_size{100};
    int max_generations{300};
    int elite_k{10};
    double mutation_rate{0.5};
    float open_threshold{50.0};   // Threshold for deciding when to open a facility
    float stop_threshold{0.0001}; // Percentage improvement threshold for stopping
    bool use_local_search{true};
    bool use_smart_leader{true};
    unsigned long long seed;
};
struct GaGeneration
{
    int generation_index;
    long best_cost;
    double avg_cost;
    long long time_total_ms;
    long long time_evolution_ms;
    long long time_localsearch_ms;
    int ls_improvements;
};

struct GaRunMetrics
{
    std::string instance_name;
    int n_facilities;
    long final_cost;
    long long total_time_ms;
    std::vector<GaGeneration> history;
};

struct SAParams
{
    double T0{1000.0};
    double Tmin{0.01};
    double alpha{0.95};
    int iters_per_T{100};
    unsigned long long seed{42};
};