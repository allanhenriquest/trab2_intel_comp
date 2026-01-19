#include "MonteCarlo.h"
#include "util/Random.h"
#include <algorithm>
#include <omp.h>
#include <limits>
#include <cmath>
#include <iostream>

using namespace std;

double MonteCarlo::expectedCost(const Instance &inst, const Solution &sol,
                                const Solution &best_deter_sol, int samples, int k, unsigned long long seed)
{
    // 1. Deterministic Fixed Costs
    double fixed_cost = 0.0;
    for (int i = 0; i < inst.n; ++i)
    {
        if (sol.openFacilities[i])
            fixed_cost += inst.opening_costs[i];
    }

    if (fixed_cost == 0.0)
        return 1e15; // Max penalty if empty

    // 2. Calculate Threshold (Max service cost in deterministic solution)
    bool has_deter_sol = false;
    double c_max = 0.0;

    for(auto assigned : sol.assigned_facility) {
        if(assigned.second > c_max)
            c_max = assigned.second;
    }

    // 3. Stochastic Simulation (Service Costs)
    double total_service_cost = 0.0;
    double var_factor = static_cast<double>(k);

    int c_max_exceed_count = 0;

    #pragma omp parallel reduction(+ : total_service_cost)
    {
        Random local_rng(seed + omp_get_thread_num());

        #pragma omp for
        for (int s = 0; s < samples; ++s)
        {
            double scenario_cost = 0.0;

            for (int j = 0; j < inst.m; ++j)
            {
                int assigned = sol.assigned_facility[j].first;
                double expected = sol.assigned_facility[j].second;

                if(expected <= 0.0)
                    continue;

                double variance = var_factor * expected;
                
                double stoch_c = local_rng.lognormal(expected, variance);

                // Apply Penalty
                if (stoch_c < numeric_limits<double>::infinity())
                {
                    if (stoch_c > c_max)
                    {
                        #pragma omp critical
                        {
                            c_max_exceed_count++;
                        }

                        // Penalty: Realized Cost + 2 * Opening Cost
                        scenario_cost += stoch_c + (2.0 * inst.opening_costs[assigned]);
                    }
                    else
                    {
                        scenario_cost += stoch_c;
                    }
                }
                else
                {
                    #pragma omp critical
                    {
                        cout << "Warning: Infinite cost sampled for client " 
                        << j << " in scenario " << s << " stoch_c: " << stoch_c << endl;
                    }
                    scenario_cost += 1e9; // Penalty for no service
                }
            }
            total_service_cost += scenario_cost;
        }
    }

    return fixed_cost + (total_service_cost / samples);
}