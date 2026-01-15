#include "SA.h"
#include <cmath>
#include <iostream>
#include <random>

using namespace std;

SA::SA(SAParams params) : params_(params) {}

Solution SA::refine(const Instance& inst, const Solution& seed, const CostEstimator& estimator) {
    mt19937 rng(params_.seed);
    uniform_real_distribution<double> dist01(0.0, 1.0);
    uniform_int_distribution<int> distN(0, inst.n - 1);

    Solution current = seed;
    current.total_cost = estimator(inst, current); 
    
    Solution best = current;
    double T = params_.T0;

    while (T > params_.Tmin) {
        for (int i = 0; i < params_.iters_per_T; ++i) {
            Solution neighbor = current;
            int facility = distN(rng);
            neighbor.openFacilities[facility] = !neighbor.openFacilities[facility];
            neighbor.ensureAtLeastOneOpen();

            neighbor.total_cost = estimator(inst, neighbor);

            double delta = neighbor.total_cost - current.total_cost;
            
            if (delta < 0) {
                current = neighbor;
                if (current.total_cost < best.total_cost) best = current;
            } else {
                if (dist01(rng) < exp(-delta / (T + 1e-9))) current = neighbor;
            }
        }
        T *= params_.alpha;
    }
    return best;
}