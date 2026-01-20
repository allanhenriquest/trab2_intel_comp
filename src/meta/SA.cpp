#include "SA.h"
#include <cmath>
#include <iostream>
#include <random>
#include <algorithm>

using namespace std;

SA::SA(SAParams params) : params_(params) {}

pair<Solution, vector<SaStep>> SA::refine(const Instance& inst, const Solution& initial_sol, const CostEstimator& estimator) {
    mt19937 rng(params_.seed);
    uniform_real_distribution<double> dist01(0.0, 1.0);
    uniform_int_distribution<int> distN(0, inst.n - 1);

    // --- CONFIGURAÇÃO SIMHEURÍSTICA ---
    // Nível Curto: Para navegação rápida (Metropolis)
    int short_samples = params_.mc_samples; 
    // Nível Longo: Para validação de élites (Best update)
    // Se short for 100, long será 2000. Garante robustez no resultado final.
    int long_samples = max(1000, short_samples * 20); 

    // 1. Inicialização
    Solution current = initial_sol;
    // Avalia o inicial com precisão curta para o loop
    current.expected_cost = estimator(inst, current, short_samples); 
    
    // O Best deve ser avaliado com PRECISÃO LONGA para ser uma âncora confiável
    Solution best = current;
    best.expected_cost = estimator(inst, best, long_samples);
    
    double T = params_.T0;
    vector<SaStep> history;
    int global_iter = 0;

    // Loop SA
    while (T > params_.Tmin) {
        for (int i = 0; i < params_.iters_per_T; ++i) {
            global_iter++;
            Solution neighbor = current;
            
            // Movimento: Flip
            int facility = distN(rng);
            neighbor.openFacilities[facility] = !neighbor.openFacilities[facility];
            neighbor.ensureAtLeastOneOpen(); 

            // AVALIAÇÃO RÁPIDA (Short Simulation)
            // Permite explorar milhares de vizinhos rapidamente
            neighbor.expected_cost = estimator(inst, neighbor, short_samples);

            // Comparação para Movimento (Short vs Short)
            // É importante comparar maçãs com maçãs (mesma variância de amostragem)
            double delta = neighbor.expected_cost - current.expected_cost;
            bool accepted = false;
            
            if (delta < 0) {
                current = neighbor;
                accepted = true;
                
                // --- MOMENTO CRÍTICO DA SIMHEURÍSTICA ---
                // Achamos um candidato que parece melhor que o Best?
                // Comparamos o Current(Short) com Best(Long).
                if (current.expected_cost < best.expected_cost) {
                    
                    // Não confiamos no 'Short'. Pode ser sorte da variância.
                    // Fazemos a PROVA REAL (Long Simulation).
                    double accurate_cost = estimator(inst, current, long_samples);
                    
                    // Se a prova real confirmar que é melhor:
                    if (accurate_cost < best.expected_cost) {
                        best = current;
                        best.expected_cost = accurate_cost; // Salvamos o custo preciso
                        
                        // Nota: Não atualizamos 'current.expected_cost' com o valor preciso
                        // para não enviesar o cálculo do delta na próxima iteração.
                        // O 'current' continua a navegar no "mundo ruidoso/rápido".
                    }
                }
            } else {
                // Critério de Metropolis padrão
                double probability = exp(-delta / (T + 1e-9));
                if (dist01(rng) < probability) {
                    current = neighbor;
                    accepted = true;
                }
            }
            
            // Log (opcional: salvar menos frequentemente para performance)
            if (global_iter % 10 == 0) {
                history.push_back({
                    global_iter, 
                    T, 
                    current.expected_cost, // Custo Short (ruidoso)
                    best.expected_cost,    // Custo Long (preciso)
                    accepted
                });
            }
        }
        T *= params_.alpha;
    }

    // Retorna o Best (que tem custo validado com long_samples)
    return {best, history};
}