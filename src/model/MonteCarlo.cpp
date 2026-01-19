#include "MonteCarlo.h"
#include "util/Random.h"
#include <algorithm>
#include <omp.h>
#include <limits>
#include <cmath>

using namespace std;

double MonteCarlo::expectedCost(const Instance& inst, const Solution& sol, 
    const Solution& best_deter_sol, int samples, int k, unsigned long long seed) 
{
    // 1. Deterministic Fixed Costs
    double fixed_cost = 0.0;
    for(int i = 0; i < inst.n; ++i) {
        if (sol.openFacilities[i]) fixed_cost += inst.opening_costs[i];
    }

    if (fixed_cost == 0.0) return 1e15; // Max penalty if empty

    // 2. Calculate Threshold (Max service cost in deterministic solution)
    double threshold = 0.0;
    bool has_deter_sol = false;

    for(bool open : best_deter_sol.openFacilities) {
        if(open) { has_deter_sol = true; break; }
    }

    if (has_deter_sol) {
        for (int j = 0; j < inst.m; ++j) {
            double min_c = numeric_limits<double>::infinity();
            for (int i = 0; i < inst.n; ++i) {
                if (best_deter_sol.openFacilities[i]) {
                    double c = inst.allocation_costs[i][j];
                    if (c < min_c) min_c = c;
                }
            }
            if (min_c < numeric_limits<double>::infinity() && min_c > threshold) {
                threshold = min_c;
            }
        }
    } else {
        threshold = numeric_limits<double>::infinity(); 
    }

    // 3. Stochastic Simulation (Service Costs)
    double total_service_cost = 0.0;
    double var_factor = static_cast<double>(k);

    #pragma omp parallel reduction(+:total_service_cost)
    {
        Random local_rng(seed + omp_get_thread_num());

        #pragma omp for
        for (int s = 0; s < samples; ++s) {
            double scenario_cost = 0.0;
            
            for (int j = 0; j < inst.m; ++j) {
                double best_cost = numeric_limits<double>::infinity();
                int best_fac = -1;

                // Find best facility for client j in this scenario
                for (int i = 0; i < inst.n; ++i) {
                    if (sol.openFacilities[i]) {
                        double mean = inst.allocation_costs[i][j];
                        double variance = var_factor * mean; 
                        
                        // Log-Normal distribution
                        double stoch_c = local_rng.lognormal(mean, variance);

                        if (stoch_c < best_cost) {
                            best_cost = stoch_c;
                            best_fac = i;
                        }
                    }
                }

                // Apply Penalty (Peidro et al., 2024)
                if (best_cost < numeric_limits<double>::infinity()) {
                    if (best_cost > threshold) {
                        // Penalty: Realized Cost + 2 * Opening Cost
                        scenario_cost += best_cost + (2.0 * inst.opening_costs[best_fac]);
                    } else {
                        scenario_cost += best_cost;
                    }
                } else {
                    scenario_cost += 1e9; // Penalty for no service
                }
            }
            total_service_cost += scenario_cost;
        }
    }

    return fixed_cost + (total_service_cost / samples);
}