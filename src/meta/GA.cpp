#include "GA.h"
#include "model/Evaluator.h"
#include "util/Timer.h"
#include <algorithm>
#include <random>
#include <omp.h>
#include <limits>
#include <numeric>
#include <iostream>

using namespace std;

// Global RNG for serial parts (Initialization, Selection, Crossover)
static mt19937 rng;

GA::GA(GAParams params, Instance instance) : params_(params), instance_(instance) {
    rng.seed(params.seed);
}

pair<vector<Solution>, RunMetrics> GA::run(const Instance& inst, int elite_k_override) {
    instance_ = inst;
    RunMetrics run_data;
    run_data.instance_name = inst.filePath;
    run_data.n_facilities = inst.n;

    Timer total_timer;
    total_timer.start();

    initializePopulation();
    evaluatePopulation();
    
    // Sort initial population
    sort(population.begin(), population.end(), [](const Solution& a, const Solution& b) {
        return a.total_cost < b.total_cost;
    });

    for (int gen = 0; gen < params_.max_generations; ++gen) {
        RunMetrics::Generation gm;
        gm.generation_index = gen;
        Timer gen_timer; 
        gen_timer.start();
        
        nextGeneration(gm);

        gen_timer.stop();
        gm.time_total_ms = gen_timer.elapsedMs();
        
        gm.best_cost = population[0].total_cost;
        double sum = 0; for(const auto& s : population) sum += s.total_cost;
        gm.avg_cost = sum / population.size();

        run_data.history.push_back(gm);
    }

    total_timer.stop();
    run_data.total_time_ms = total_timer.elapsedMs();
    run_data.final_cost = population[0].total_cost;

    int k = (elite_k_override > 0) ? elite_k_override : params_.elite_k;
    return {selectElite(k), run_data};
}

// --- SMART INITIALIZATION ---
void GA::initializePopulation() {
    population.clear();
    double p_open = max(0.02, 10.0 / (double)instance_.n);
    uniform_real_distribution<double> dist(0.0, 1.0);

    // 1. Inject Greedy Solution (The "Smart Leader")
    Solution greedy = generateGreedySolution();
    population.push_back(greedy);

    // 2. Fill the rest with Random Solutions
    for(int i = 1; i < params_.pop_size; ++i) {
        Solution sol(instance_.n, instance_.m);
        for(int j=0; j < instance_.n; ++j) {
            sol.openFacilities[j] = (dist(rng) < p_open);
        }
        sol.ensureAtLeastOneOpen();
        population.push_back(sol);
    }
}

Solution GA::generateGreedySolution() {
    Solution sol(instance_.n, instance_.m);
    
    // Step 1: Pick the single best facility to start
    int best_first = -1;
    double best_first_cost = numeric_limits<double>::infinity();

    for(int i=0; i<instance_.n; ++i) {
        double current = instance_.opening_costs[i];
        for(int j=0; j<instance_.m; ++j) {
            current += instance_.allocation_costs[i][j];
        }
        if (current < best_first_cost) {
            best_first_cost = current;
            best_first = i;
        }
    }
    
    sol.openFacilities[best_first] = true;
    Evaluator::evaluateFull(instance_, sol);

    // Step 2: Iteratively add facilities
    bool improving = true;
    while(improving) {
        improving = false;
        int best_candidate = -1;
        double current_best_cost = sol.total_cost;

        for(int i=0; i<instance_.n; ++i) {
            if(!sol.openFacilities[i]) {
                Evaluator::openFacility(i, instance_, sol);
                if (sol.total_cost < current_best_cost - 1e-4) {
                    current_best_cost = sol.total_cost;
                    best_candidate = i;
                }
                Evaluator::closeFacility(i, instance_, sol);
            }
        }

        if (best_candidate != -1) {
            Evaluator::openFacility(best_candidate, instance_, sol);
            improving = true;
        }
    }
    return sol;
}

void GA::evaluatePopulation() {
    #pragma omp parallel for
    for (int i = 0; i < (int)population.size(); ++i) {
        Evaluator::evaluateFull(instance_, population[i]);
    }
}

void GA::nextGeneration(RunMetrics::Generation& current_metrics) { 
    Timer evo_timer; evo_timer.start();

    vector<Solution> nextPop;
    int elites = params_.elite_k;
    
    // Elitism
    for (int i = 0; i < elites; ++i) nextPop.push_back(population[i]);

    int needed = params_.pop_size - elites;
    vector<Solution> offspring(needed, Solution(instance_.n, instance_.m));
    
    uniform_int_distribution<int> distIdx(0, params_.pop_size - 1);
    uniform_int_distribution<int> distN(0, instance_.n - 1);
    uniform_real_distribution<double> dist01(0.0, 1.0);

    // SERIAL PART: Selection & Mutation (Safe to use global rng)
    for(int i=0; i<needed; ++i) {
        // Tournament Selection
        const Solution& p1 = population[distIdx(rng)];
        const Solution& p2 = population[distIdx(rng)];
        const Solution& parent1 = (p1.total_cost < p2.total_cost) ? p1 : p2;
        
        const Solution& p3 = population[distIdx(rng)];
        const Solution& p4 = population[distIdx(rng)];
        const Solution& parent2 = (p3.total_cost < p4.total_cost) ? p3 : p4;

        // Uniform Crossover
        for(int j=0; j<instance_.n; ++j) {
            offspring[i].openFacilities[j] = (dist01(rng) < 0.5) ? parent1.openFacilities[j] : parent2.openFacilities[j];
        }
        
        // Mutation (Flip & Swap)
        if (dist01(rng) < params_.mutation_rate) {
            if (dist01(rng) < 0.5) {
                // Flip
                int idx = distN(rng);
                offspring[i].openFacilities[idx] = !offspring[i].openFacilities[idx];
            } else {
                // Swap
                vector<int> openIndices, closedIndices;
                for(int f=0; f<instance_.n; ++f) {
                    if (offspring[i].openFacilities[f]) openIndices.push_back(f);
                    else closedIndices.push_back(f);
                }
                if (!openIndices.empty() && !closedIndices.empty()) {
                    uniform_int_distribution<int> randOpen(0, openIndices.size()-1);
                    uniform_int_distribution<int> randClosed(0, closedIndices.size()-1);
                    
                    int f_open = openIndices[randOpen(rng)];
                    int f_closed = closedIndices[randClosed(rng)];
                    
                    offspring[i].openFacilities[f_open] = false;
                    offspring[i].openFacilities[f_closed] = true;
                }
            }
        }

        offspring[i].ensureAtLeastOneOpen();
        Evaluator::evaluateFull(instance_, offspring[i]);
    }
    
    evo_timer.stop();
    current_metrics.time_evolution_ms = evo_timer.elapsedMs();

    Timer ls_timer; ls_timer.start();
    int improvements = 0;
    
    // PARALLEL PART: Local Search
    // FIX: Pass a unique, deterministic seed offset to each thread
    #pragma omp parallel for reduction(+:improvements)
    for(int i=0; i<needed; ++i) {
        // Unique Seed = Index + (Generation * LargeNumber)
        int unique_seed_modifier = i + (current_metrics.generation_index * 10000);
        
        if (optimizeSolution(offspring[i], unique_seed_modifier)) {
            improvements++;
        }
    }
    
    ls_timer.stop();
    current_metrics.time_localsearch_ms = ls_timer.elapsedMs();
    current_metrics.ls_improvements = improvements;

    nextPop.insert(nextPop.end(), offspring.begin(), offspring.end());
    population = nextPop;
    
    sort(population.begin(), population.end(), [](const Solution& a, const Solution& b) {
        return a.total_cost < b.total_cost;
    });
}

// THREAD-SAFE LOCAL SEARCH
bool GA::optimizeSolution(Solution& sol, int seed_offset) {
    bool overall_improvement = false;
    bool improved_this_round = true;
    
    // Initialize Local RNG for this thread/solution
    mt19937 local_rng(params_.seed + seed_offset);

    vector<int> indices(instance_.n);
    iota(indices.begin(), indices.end(), 0);
    
    while (improved_this_round) {
        improved_this_round = false;
        
        // Use local_rng to shuffle deterministically
        shuffle(indices.begin(), indices.end(), local_rng);

        for (int facility : indices) {
            double old_cost = sol.total_cost;
            
            if (sol.openFacilities[facility]) {
                 // Try closing
                 int count = 0;
                 for(auto b : sol.openFacilities) if(b) count++;
                 if (count > 1) {
                    Evaluator::closeFacility(facility, instance_, sol);
                 } else {
                    continue; 
                 }
            } else {
                 // Try opening
                 Evaluator::openFacility(facility, instance_, sol);
            }

            if (sol.total_cost < old_cost - 1e-4) {
                improved_this_round = true;
                overall_improvement = true;
                break; // Restart scan
            } else {
                // Revert
                if (sol.openFacilities[facility]) {
                    Evaluator::closeFacility(facility, instance_, sol);
                } else {
                    Evaluator::openFacility(facility, instance_, sol);
                }
                sol.total_cost = old_cost; 
            }
        }
    }
    return overall_improvement;
}

vector<Solution> GA::selectElite(int k) const { 
    vector<Solution> result;
    for(int i=0; i < min((int)population.size(), k); ++i) result.push_back(population[i]);
    return result; 
}