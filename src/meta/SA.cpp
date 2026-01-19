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

    Solution current = initial_sol;
    current.expected_cost = estimator(inst, current); 
    
    Solution best = current;
    double T = params_.T0;
    
    vector<SaStep> history; // <--- Log
    int global_iter = 0;

    while (T > params_.Tmin) {
        for (int i = 0; i < params_.iters_per_T; ++i) {
            global_iter++;
            Solution neighbor = current;
            
            // Movimento: Flip
            int facility = distN(rng);
            neighbor.openFacilities[facility] = !neighbor.openFacilities[facility];
            neighbor.ensureAtLeastOneOpen(); 

            neighbor.expected_cost = estimator(inst, neighbor);

            double delta = neighbor.expected_cost - current.expected_cost;
            bool accepted = false;
            
            if (delta < 0) {
                current = neighbor;
                accepted = true;
                if (current.expected_cost < best.expected_cost) {
                    best = current;
                }
            } else {
                double probability = exp(-delta / (T + 1e-9));
                if (dist01(rng) < probability) {
                    current = neighbor;
                    accepted = true;
                }
            }
            
            // Log a cada X iterações para não gerar arquivos gigantes
            // (Ou logar sempre se iters_per_T for pequeno)
            history.push_back({
                global_iter, 
                T, 
                current.expected_cost, 
                best.expected_cost, 
                accepted
            });
        }
        T *= params_.alpha;
    }

    return {best, history};
}