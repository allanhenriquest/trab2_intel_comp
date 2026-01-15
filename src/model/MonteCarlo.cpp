#include "MonteCarlo.h"
#include <random>
#include <algorithm>
#include <omp.h>
#include <limits>

using namespace std;

double MonteCarlo::expectedCost(const Instance& inst, const Solution& sol, 
    const Solution& best_deter_sol, int samples, int k, unsigned long long seed) 
{
    // 1. Fixed Costs (Deterministic)
    double fixed_cost = 0.0;
    for(int i = 0; i < inst.n; ++i) {
        // Correctly accessing the vector 'opening_costs'
        if (sol.openFacilities[i]) fixed_cost += inst.opening_costs[i];
    }

    // 2. Service Costs (Stochastic Simulation)
    double total_service_cost = 0.0;
    
    #pragma omp parallel reduction(+:total_service_cost)
    {
        mt19937 local_rng(seed + omp_get_thread_num());
        uniform_real_distribution<double> dist(0.0, 1.0);

        #pragma omp for
        for (int s = 0; s < samples; ++s) {
            double scenario_cost = 0.0;
            
            for (int j = 0; j < inst.n; ++j) {
                // 50% chance client exists
                if (dist(local_rng) < 0.5) { 
                    double min_dist = numeric_limits<double>::infinity();
                    for (int i = 0; i < inst.n; ++i) {
                        if (sol.openFacilities[i]) {
                            double c = inst.allocation_costs[i][j];
                            if (c < min_dist) min_dist = c;
                        }
                    }
                    if (min_dist < numeric_limits<double>::infinity()) {
                        scenario_cost += min_dist;
                    } else {
                        scenario_cost += 1e9; 
                    }
                }
            }
            total_service_cost += scenario_cost;
        }
    }

    return fixed_cost + (total_service_cost / samples);
}