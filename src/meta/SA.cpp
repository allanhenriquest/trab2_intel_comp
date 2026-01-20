#include "SA.h"
#include "model/Evaluator.h" // <--- Importante: Incluir Evaluator
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

    // Configuração de Amostras
    int short_samples = params_.mc_samples; 
    int long_samples = max(1000, short_samples * 20); 

    Solution current = initial_sol;
    
    // CORREÇÃO: Garantir que a solução inicial tenha cache atualizado
    Evaluator::evaluateFull(inst, current); 
    current.expected_cost = estimator(inst, current, short_samples); 
    
    Solution best = current;
    best.expected_cost = estimator(inst, best, long_samples);
    
    double T = params_.T0;
    vector<SaStep> history;
    int global_iter = 0;

    while (T > params_.Tmin) {
        for (int i = 0; i < params_.iters_per_T; ++i) {
            global_iter++;
            
            // ==========================================================
            // CORREÇÃO DO BUG DA INSTALAÇÃO FANTASMA
            // ==========================================================
            Solution neighbor = current; // Copia a solução atual
            int facility = distN(rng);   // Escolhe instalação para inverter

            // Em vez de apenas inverter o booleano, usamos o Evaluator
            // para garantir que os clientes sejam realocados corretamente.
            if (neighbor.openFacilities[facility]) {
                // Tenta fechar (só fecha se não for a única aberta)
                if (neighbor.num_open_facilities > 1) {
                    Evaluator::closeFacility(facility, inst, neighbor);
                } else {
                    // Se for a única, não pode fechar. Pula iteração ou tenta abrir outra.
                    // Para simplicidade, abrimos outra aleatória antes de fechar esta,
                    // ou simplesmente ignoramos o movimento.
                    int other = distN(rng);
                    if(other != facility) Evaluator::openFacility(other, inst, neighbor);
                }
            } else {
                // Tenta abrir
                Evaluator::openFacility(facility, inst, neighbor);
            }
            // ==========================================================

            // Agora 'neighbor' tem FixedCost atualizado E Assignments atualizados.
            // O MonteCarlo lerá dados reais.
            neighbor.expected_cost = estimator(inst, neighbor, short_samples);

            double delta = neighbor.expected_cost - current.expected_cost;
            bool accepted = false;
            
            if (delta < 0) {
                current = neighbor;
                accepted = true;
                
                if (current.expected_cost < best.expected_cost) {
                    // Prova Real (Long Simulation)
                    double accurate_cost = estimator(inst, current, long_samples);
                    if (accurate_cost < best.expected_cost) {
                        best = current;
                        best.expected_cost = accurate_cost; 
                    }
                }
            } else {
                double probability = exp(-delta / (T + 1e-9));
                if (dist01(rng) < probability) {
                    current = neighbor;
                    accepted = true;
                }
            }
            
            if (global_iter % 50 == 0) { // Log menos frequente
                history.push_back({
                    global_iter, T, current.expected_cost, best.expected_cost, accepted
                });
            }
        }
        T *= params_.alpha;
    }

    return {best, history};
}