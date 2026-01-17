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

pair<vector<Solution>, GaRunMetrics> GA::run(const Instance& inst, int elite_k_override) 
{
    instance_ = inst;
    GaRunMetrics run_data;
    run_data.instance_name = inst.filePath;
    run_data.n_facilities = inst.n;

    Timer total_timer;
    total_timer.start();

    initializePopulation(params_.use_smart_leader, params_.open_threshold);;
    evaluatePopulation();
    sort(population.begin(), population.end(), [](const Solution& a, const Solution& b) {
        return a.total_cost < b.total_cost;
    });

    long old_cost = 0;
    int convergence_counter = 0;
    for (int gen = 0; gen < params_.max_generations; ++gen) {
        GaGeneration gm;
        gm.generation_index = gen;
        Timer gen_timer; 
        gen_timer.start();
        
        nextGeneration(gm, params_.use_local_search);

        gen_timer.stop();
        gm.time_total_ms = gen_timer.elapsedMs();
        
        gm.best_cost = population[0].total_cost;
        double sum = 0; for(const auto& s : population) sum += s.total_cost;
        gm.avg_cost = sum / population.size();
        
        run_data.history.push_back(gm);

        if(gen > 0 && (abs(gm.best_cost - old_cost)/(double)old_cost) < params_.stop_threshold)
            convergence_counter++;
        else
            convergence_counter = 0;

        if(convergence_counter >= 10) {
            break;
        }

        old_cost = gm.best_cost;
    }

    total_timer.stop();
    run_data.total_time_ms = total_timer.elapsedMs();
    run_data.final_cost = population[0].total_cost;

    int k = (elite_k_override > 0) ? elite_k_override : params_.elite_k;
    return {selectElite(k), run_data};
}

void GA::initializePopulation(bool use_smart_leader, float open_threshold) {
    double p_open = max(0.02, open_threshold / (double)instance_.n);
    uniform_real_distribution<double> dist(0.0, 1.0);
    
    population.clear();
    population.resize(params_.pop_size);

    for (int i = 0; i < params_.pop_size; ++i)
    {   
        Solution sol(instance_.n, instance_.m);
        for (int j = 0; j < instance_.n; ++j)
        {
            if (dist(rng) < p_open)
            {
                sol.openFacilities[j] = true;
                sol.num_open_facilities++;
            }
        }
        sol.ensureAtLeastOneOpen();
        population[i] = sol;
    }

    // Inject Greedy Solution (The "Smart Leader")
    if(use_smart_leader) {
        Solution greedy = generateGreedySolution();
        population[0] = greedy;
    }
}

Solution GA::generateGreedySolution() {
    Solution sol(instance_.n, instance_.m);
    
    // Step 1: Pick the single best facility to start
    int best_first = -1;
    long best_first_cost = numeric_limits<long>::max();

    for(int i=0; i<instance_.n; ++i) {
        long current = instance_.opening_costs[i];
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
        long current_best_cost = sol.total_cost;

        // Parallel search for best facility to open
        int local_best = -1;
        long local_cost = current_best_cost;
        
        #pragma omp parallel
        {
            int thread_best = -1;
            long thread_cost = current_best_cost;
            
            #pragma omp for nowait
            for(int i=0; i<instance_.n; ++i) {
                if(!sol.openFacilities[i]) {
                    long new_cost = sol.total_cost + Evaluator::delta(i, instance_, sol);
                    if (new_cost < thread_cost) {
                        thread_cost = new_cost;
                        thread_best = i;
                    }
                }
            }
            
            #pragma omp critical
            {
                if (thread_best != -1 && thread_cost < local_cost) {
                    local_cost = thread_cost;
                    local_best = thread_best;
                }
            }
        }
        
        if (local_best != -1) {
            current_best_cost = local_cost;
            best_candidate = local_best;
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

void GA::nextGeneration(GaGeneration& current_metrics, bool use_local_search) { 
    Timer evo_timer; evo_timer.start();

    vector<Solution> nextPop;
    int elites = params_.elite_k;
    
    // Elitism
    nextPop.reserve(params_.pop_size);
    nextPop.insert(nextPop.end(), population.begin(), population.begin() + elites);

    int needed = params_.pop_size - elites;
    vector<Solution> childreen;
    childreen.reserve(needed);
    
    uniform_int_distribution<int> distIdx(0, params_.pop_size - 1);
    uniform_int_distribution<int> distN(0, instance_.n - 1);
    uniform_real_distribution<double> dist01(0.0, 1.0);

    for(int i=0; i<needed; ++i) {
        // Create new child
        childreen.emplace_back(instance_.n, instance_.m);
        
        // Tournament Selection
        const Solution& p1 = population[distIdx(rng)];
        const Solution& p2 = population[distIdx(rng)];
        const Solution& parent1 = (p1.total_cost < p2.total_cost) ? p1 : p2;
        
        const Solution& p3 = population[distIdx(rng)];
        const Solution& p4 = population[distIdx(rng)];
        const Solution& parent2 = (p3.total_cost < p4.total_cost) ? p3 : p4;

        // Uniform Crossover
        for(int j=0; j<instance_.n; ++j) {
            bool should_open = (dist01(rng) < 0.5) ? parent1.openFacilities[j] : parent2.openFacilities[j];
            childreen[i].openFacilities[j] = should_open;
            if (should_open) childreen[i].num_open_facilities++;
        }
        
        // Mutation (Flip & Swap)
        if (dist01(rng) < params_.mutation_rate) {
            if (dist01(rng) < 0.5) {
                // Flip
                int idx = distN(rng);
                if (childreen[i].openFacilities[idx]) {
                    childreen[i].openFacilities[idx] = false;
                    childreen[i].num_open_facilities--;
                } else {
                    childreen[i].openFacilities[idx] = true;
                    childreen[i].num_open_facilities++;
                }
            } else {
                // Swap
                vector<int> openIndices, closedIndices;
                openIndices.reserve(instance_.n);
                closedIndices.reserve(instance_.n);
                
                for(int f=0; f<instance_.n; ++f) {
                    if (childreen[i].openFacilities[f]) 
                        openIndices.push_back(f);
                    else 
                        closedIndices.push_back(f);
                }
                if (!openIndices.empty() && !closedIndices.empty()) {
                    uniform_int_distribution<int> randOpen(0, openIndices.size()-1);
                    uniform_int_distribution<int> randClosed(0, closedIndices.size()-1);
                    
                    int f_open = openIndices[randOpen(rng)];
                    int f_closed = closedIndices[randClosed(rng)];
                    
                    childreen[i].openFacilities[f_open] = false;
                    childreen[i].openFacilities[f_closed] = true;
                    // No change to num_open_facilities (swap keeps count same)
                }
            }
        }

        childreen[i].ensureAtLeastOneOpen();
    }
    
    #pragma omp parallel for
    for(int i=0; i<needed; ++i) {
        Evaluator::evaluateFull(instance_, childreen[i]);
    }
    
    evo_timer.stop();
    current_metrics.time_evolution_ms = evo_timer.elapsedMs();

    if(use_local_search){
        localSearch(childreen, current_metrics, needed);
    } else {
        current_metrics.time_localsearch_ms = 0.0;
        current_metrics.ls_improvements = 0;
    }

    nextPop.insert(nextPop.end(), childreen.begin(), childreen.end());
    population = nextPop;
    
    sort(population.begin(), population.end(), [](const Solution& a, const Solution& b) {
        return a.total_cost < b.total_cost;
    });
}

void GA::localSearch(vector<Solution>& childreen, GaGeneration& current_metrics, int needed)
{
    Timer ls_timer; ls_timer.start();
    int improvements = 0;
    
    #pragma omp parallel for reduction(+:improvements)
    for(int i=0; i<needed; ++i) {
        // Unique Seed = Index + (Generation * LargeNumber)
        int unique_seed_modifier = i + (current_metrics.generation_index * 10000);
        
        if (optimizeSolution(childreen[i], unique_seed_modifier)) {
            improvements++;
        }
    }
    
    ls_timer.stop();
    current_metrics.time_localsearch_ms = ls_timer.elapsedMs();
    current_metrics.ls_improvements = improvements;
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
            // Save state before modification (Otimização 2)
            long old_cost = sol.total_cost;
            bool was_open = sol.openFacilities[facility];
            vector<pair<int, long>> old_assignments = sol.assigned_facility;
            
            if (sol.openFacilities[facility]) {
                 // Try closing (using cached count)
                 if (sol.num_open_facilities > 1) {
                    Evaluator::closeFacility(facility, instance_, sol);
                 } else {
                    continue; 
                 }
            } else {
                 // Try opening
                 Evaluator::openFacility(facility, instance_, sol);
            }

            if (sol.total_cost < old_cost) {
                improved_this_round = true;
                overall_improvement = true;
                break; // Restart scan
            } else {
                // Fast revert using saved state
                sol.openFacilities[facility] = was_open;
                sol.total_cost = old_cost;
                sol.assigned_facility = old_assignments;
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