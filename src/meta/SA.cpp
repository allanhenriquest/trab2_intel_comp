#include "SA.h"
#include <cmath>
#include <iostream>
#include <random>
#include <algorithm>

using namespace std;

SA::SA(SAParams params) : params_(params) {}

Solution SA::refine(const Instance& inst, const Solution& seed, const CostEstimator& estimator) {
    // Initialize RNG
    mt19937 rng(params_.seed);
    uniform_real_distribution<double> dist01(0.0, 1.0);
    uniform_int_distribution<int> distN(0, inst.n - 1);

    // Initial evaluation
    Solution current = seed;
    // Note: If using stochastic estimator, this will run a short simulation
    current.total_cost = estimator(inst, current); 
    
    Solution best = current;
    double T = params_.T0;

    // Main SA Loop
    while (T > params_.Tmin) {
        for (int i = 0; i < params_.iters_per_T; ++i) {
            Solution neighbor = current;
            
            // Neighborhood move: Flip status of a random facility
            // This allows the SA to add facilities (safety against penalty) 
            // or remove them (cost efficiency)
            int facility = distN(rng);
            neighbor.openFacilities[facility] = !neighbor.openFacilities[facility];
            
            // Critical: Avoid solutions with 0 open facilities (invalid for UFLP)
            neighbor.ensureAtLeastOneOpen(); 

            // Calculate cost (Simulation happens here if estimator is stochastic)
            neighbor.total_cost = estimator(inst, neighbor);

            double delta = neighbor.total_cost - current.total_cost;
            
            // Acceptance criteria (Metropolis)
            if (delta < 0) {
                // Improvement: always accept
                current = neighbor;
                if (current.total_cost < best.total_cost) {
                    best = current;
                }
            } else {
                // Worsening: accept with probability
                // Adding 1e-9 to T prevents division by zero
                double probability = exp(-delta / (T + 1e-9));
                if (dist01(rng) < probability) {
                    current = neighbor;
                }
            }
        }
        // Cooling schedule
        T *= params_.alpha;
    }

    return best;
}