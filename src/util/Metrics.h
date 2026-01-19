#pragma once
#include <vector>
#include <string>

using namespace std;

struct GAParams
{
    bool test_mode{true};
    int pop_size{100};
    int max_generations{300};
    float elite_ratio{0.1};
    int elite_count{10};
    double mutation_rate{0.001};
    float open_threshold{50.0};   // Threshold for deciding when to open a facility
    float stop_threshold{0.0001}; // Percentage improvement threshold for stopping
    int max_convergence{20};
    int max_duplicates{5};
    bool use_local_search{true};
    bool use_smart_leader{true};
    unsigned long long seed;
};

struct SAParams
{
    bool solve{true};
    double T0{100.0};
    double Tmin{1.0};
    double alpha{0.95};
    int iters_per_T{5};

    int mc_samples{1000};
    int mc_k{5};

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
    int avg_open_facilities;
    int duplicates;

    // --- NOVOS CAMPOS PARA DIVERSIDADE ---
    double cost_std_dev; // Desvio Padrão dos Custos (Diversidade Fenotípica)
    double unique_ratio; // % de Indivíduos Únicos na População (Diversidade Genotípica)
};

struct GaRunMetrics
{
    std::string instance_name;
    int n_facilities;
    long final_cost;
    long total_time_ms;
    std::vector<GaGeneration> history;

};
