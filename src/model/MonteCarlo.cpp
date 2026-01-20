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
    // 1. Custo Fixo Determinístico (Não muda com a incerteza)
    double fixed_cost = 0.0;
    for (int i = 0; i < inst.n; ++i)
    {
        if (sol.openFacilities[i])
            fixed_cost += inst.opening_costs[i];
    }

    if (fixed_cost == 0.0)
        return 1e15; // Penalidade máxima se nenhuma fábrica estiver aberta

    // 2. Calcular Threshold (c_max) baseado na Solução Determinística (OBD)
    // CORREÇÃO: Usar o Percentil 95 em vez do Máximo Absoluto.
    // O Máximo Absoluto é muito "frouxo" (se houver um outlier de custo alto na OBD, 
    // ele eleva o teto para todos, impedindo penalidades).
    // O Percentil 95 força que a cauda da distribuição sofra penalidades.
    double c_max = 0.0;
    
    if (!best_deter_sol.assigned_facility.empty()) {
        vector<double> costs;
        costs.reserve(inst.m);
        for(const auto& p : best_deter_sol.assigned_facility) {
            costs.push_back(static_cast<double>(p.second));
        }
        
        // Ordena para achar o percentil
        sort(costs.begin(), costs.end());
        
        // Pega o valor que cobre 95% dos casos. Os 5% piores da OBD já estão "no limite".
        // Qualquer variação estocástica neles vai estourar o c_max e gerar multa.
        int idx = static_cast<int>(costs.size() * 0.95);
        if(idx >= costs.size()) idx = costs.size() - 1;
        c_max = costs[idx];
        
    } else {
        // Fallback
        c_max = 0.0; 
        for(auto assigned : sol.assigned_facility) {
             if(assigned.second > c_max) c_max = assigned.second;
        }
    }
    
    // Garante um mínimo técnico para evitar c_max = 0
    if(c_max < 1.0) c_max = 1.0;

    // 3. Simulação Estocástica (Custos de Serviço)
    double total_service_cost = 0.0;
    double var_factor = static_cast<double>(k);

    #pragma omp parallel reduction(+ : total_service_cost)
    {
        Random local_rng(seed + omp_get_thread_num());

        #pragma omp for
        for (int s = 0; s < samples; ++s)
        {
            double scenario_cost = 0.0;

            for (int j = 0; j < inst.m; ++j)
            {
                // Dados da solução atual (candidata)
                int assigned_facility_idx = sol.assigned_facility[j].first;
                double expected_cost = sol.assigned_facility[j].second;

                if(expected_cost <= 0.0)
                    continue;

                // Modelo Log-Normal
                // Variance = k * Mean (Modelo Linear padrão da literatura)
                double variance = var_factor * expected_cost; 
                
                double stoch_c = local_rng.lognormal(expected_cost, variance);

                // Aplica Penalidade baseada no c_max da OBD
                if (stoch_c < numeric_limits<double>::infinity())
                {
                    if (stoch_c > c_max)
                    {
                        // Penalidade Severa: Custo Real + 2x Custo de Abertura
                        // Isso garante que Stoch > Deterministic quando há violações
                        scenario_cost += stoch_c + (0.315 * inst.opening_costs[assigned_facility_idx]);
                    }
                    else
                    {
                        scenario_cost += stoch_c;
                    }
                }
                else
                {
                    scenario_cost += 1e9; // Segurança numérica
                }
            }
            total_service_cost += scenario_cost;
        }
    }

    return fixed_cost + (total_service_cost / samples);
}