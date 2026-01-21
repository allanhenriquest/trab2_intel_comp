#include "MonteCarlo.h"
#include "util/Random.h"
#include <algorithm>
#include <omp.h>
#include <limits>
#include <cmath>
#include <iostream>
#include <vector>

using namespace std;

double MonteCarlo::expectedCost(const Instance &inst, const Solution &sol,
                                const Solution &best_deter_sol, int samples, int k, unsigned long long seed)
{
    double fixed_cost = 0.0;
    for (int i = 0; i < inst.n; ++i) {
        if (sol.openFacilities[i])
            fixed_cost += inst.opening_costs[i];
    }

    if (fixed_cost == 0.0) return 1e15; // Penalidade se vazio

    // COMPATIBILIDADE COM ARTIGO (Peidro et al., 2024): DEFINIÇÃO DE THRESHOLD
    // O c_max é o MAIOR custo de alocação presente na Melhor Solução Determinística (OBD).
    double c_max = 0.0;
    
    // Varre todas as alocações da OBD para achar o pior caso (Max c_ij)
    if (!best_deter_sol.assigned_facility.empty()) {
        for(const auto& p : best_deter_sol.assigned_facility) {
            double cost = static_cast<double>(p.second);
            if (cost > c_max) c_max = cost;
        }
    } else {
        // Fallback seguro se OBD estiver vazia (não deve acontecer)
        c_max = std::numeric_limits<double>::max(); 
    }
    
    // Evita c_max zero em casos degenerados
    if(c_max < 1.0) c_max = 1.0; 

    double total_service_cost = 0.0;
    double var_factor = static_cast<double>(k); // k=5, 10, 20 define a variância

    #pragma omp parallel reduction(+ : total_service_cost)
    {
        Random local_rng(seed + omp_get_thread_num());

        #pragma omp for
        for (int scenario = 0; scenario < samples; ++scenario)
        {
            double scenario_cost = 0.0;

            for (int j = 0; j < inst.m; ++j)
            {
                int facility_idx = sol.assigned_facility[j].first;
                double mean_cost = sol.assigned_facility[j].second;

                if(mean_cost <= 0.0) continue;

                // Modelo Log-Normal (Variância = k * Média)
                double variance = var_factor * mean_cost; 
                double stoch_c = local_rng.lognormal(mean_cost, variance);

                // COMPATIBILIDADE COM ARTIGO: REGRA DE PENALIDADE
                // Se o custo estocástico exceder c_max (da OBD), aplica-se penalidade.
                if (stoch_c > c_max) {
                    scenario_cost += stoch_c + (2.0 * inst.opening_costs[facility_idx]);
                } else {
                    scenario_cost += stoch_c;
                }
            }
            total_service_cost += scenario_cost;
        }
    }

    return fixed_cost + (total_service_cost / samples);
}